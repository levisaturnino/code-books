<?php
/**
 * Classe para gerenciamento de menus do sistema
 *
 */
class menu extends BASE {
	public function __construct(BancoDados $_conn) {
		parent::__construct($_conn);
		$this->_nome_tabela = 'menu';
		$this->addCampo(new inteiro("MENU_ID","C�digo",10,null,null,false,false,true,null,null,null,true));
		$this->addCampo(new string("MENU_DESC","Descri��o",30,null,null,true,true,true,5,null,null,false,$_conn));
		$this->addCampo(new string("MENU_PROG","Programa",120,null,null,true,true,true,null,null,null,false,$_conn));
		$this->addCampo(new string("MENU_ICON","Imagem relacionada",120,null,null,true,true,false,null,null,null,false,$_conn));
		$this->addCampo(new inteiro("MENU_PAI","Menu Relacionado(Pai)",10,null,null,true,true,false,null,null,null,false,$_conn));
		$this->getCampo("MENU_PROG")->setComportamento_form("select");
		$_programas = $this->getListaArquivos(Array('valor'=>'[menu]','label'=>'[MENU]'));
		$_diretorios = include_once("framework/includes/configmenu.inc");
		if(is_array($_diretorios)) {
			foreach($_diretorios as $_titulo=>$_dir) {
				$_programas = array_merge($_programas,$this->getListaArquivos(null,$_dir,Array("php5"),"[{$_titulo}] ",true));
			}
		}
		$this->getCampo("MENU_PROG")->setValor_Fixo($_programas);		
		$this->getCampo("MENU_ICON")->setComportamento_form("select");
		$this->getCampo("MENU_ICON")->setValor_Fixo($this->getListaArquivos(
															Array('valor'=>'','label'=>'Nenhum'),
															"framework/imagens/",
															Array("gif","png","jpg","tif","bmp")
													));		
		$this->getCampo("MENU_PAI")->setComportamento_form("select");
		$this->getCampo("MENU_PAI")->setValor_Fixo($this->buscaMenusPai(($_GET['ACAO']=='ALT' ? $_GET['MENU_ID'] : 0)));
		$this->getCampo("MENU_ICON")->setFiltro(false);
		$this->getCampo("MENU_PROG")->setFiltro(false);
		
	}
	
	protected function validaValor($_campo,$_valor) {
		if($_campo=='MENU_PAI'&&$_valor=='-') {
			return false;
		}
		return true;
	}

	protected function preFiltro() {
		$_opcoes = array_merge(Array(Array('label'=>"-",'valor'=>'-')),$this->getCampo("MENU_PAI")->getValor_Fixo());
		$this->getCampo("MENU_PAI")->setValor_Fixo($_opcoes);
	}
	
	/**
	 * Retorna uma lista de Menus que podem ser Pai de outro menu
	 * 
	 *
	 * @param integer $_item_atual
	 */
	protected function buscaMenusPai($_item_atual=0) {
		$_strSQL = "SELECT MENU_ID,MENU_DESC FROM {$this->_nome_tabela} " . 
				   "WHERE MENU_ID<>{$_item_atual} AND MENU_PROG='[menu]' " . 
				   "ORDER BY MENU_PAI";
		$_menus = Array(Array("label"=>"Nenhum","valor"=>-1));
		if($this->_conn->executaSQL($_strSQL)!==false&&$this->_conn->getNumRows()>0) {
			while(($_dados=$this->_conn->proximo())!==false) {
				$_menus[] = Array('valor'=>$_dados['MENU_ID'],'label'=>$_dados['MENU_DESC']);
			}
		}
		return $_menus;
	}
	
	/**
	 * Retorna uma lista de arquivos com as extens�es informadas
	 *
	 * @param Array $_ext
	 */
	protected function getListaArquivos($_fixo=null,$_diretorio="./",
										Array $_ext=Array("php5"),$_sufixo="",
										$_inclui_diretorio=false) {
		$_dir = dir($_diretorio);
		$_arquivos = Array();
		while (($_l=$_dir->read())!==FALSE) {
			if(eregi("^.*\.(.{1,})$",$_l,$_f)!==false) {
				if(in_array($_f[1],$_ext)) {
					$_arquivos[] = $_l;
				}
			}
		}
		$_dir->close();
		ksort($_arquivos);
		$_opcoes = Array();
		if($_fixo!==null) {
			$_opcoes[] = $_fixo;
		}
		foreach($_arquivos as $_arq) {
			$_opcoes[] = Array('label'=>"{$_sufixo}{$_arq}",
							   'valor'=>($_inclui_diretorio===true ? $_diretorio : "") . $_arq);
		}
		return $_opcoes;
		
	}
	
	/**
	 * Retorna o comando SQL para gera��o do 
	 * conjunto de dados resultante da busca AJAX
	 *
	 * @return string
	 */
	public function montaSQLAjax() {
		return "SELECT MENU_ID,MENU_DESC FROM {$this->_nome_tabela} WHERE " . 
			 	" MENU_ID=" . ((int) $_POST['valor']) . 
				" OR LOWER(MENU_DESC) LIKE '%" . strtolower($_POST['valor']) . "%' " . 
				"ORDER BY MENU_DESC";
	}
	
	/**
	 * Retorna o comando SQL necess�rio para
	 * a busca do pr�ximo c�digo
	 *
	 * @return string
	 */
	public function montaSQLProximoCodigo() {
		return "SELECT MAX(MENU_ID) AS CODIGO FROM {$this->_nome_tabela}";
	}
	
	/**
	 * Busca o pr�ximo c�digo para o menu
	 *
	 * @return mixed
	 */
	public function incluir() {
		if(($_cod=$this->getProximoCodigo(true))=='[ERRO]') {
			return false;
		} else {
			$this->getCampo("MENU_ID")->setValor($_cod);
			$this->getCampo("MENU_ID")->setIncluir(true);
			return parent::incluir();
		}
	}
	
	/**
	 * Antes de alterar precisamos verificar se existe alguma inconsistencia
	 *
	 * @return mixed
	 */
	public function alterar() {
		if($_POST['MENU_PROG_OLD']=='[menu]'&&$_POST['MENU_PROG']!='[menu]') {
			if($this->_conn->executaSQL($this->montaSELECT("COUNT(*) AS CONT","MENU_PAI={$_POST['MENU_ID']}"))!==false) {
				$_dados = $this->_conn->proximo();				
				if($_dados['CONT']>0) {
					$this->_msgextra = 'Menu possui submenus e n�o pode deixar de ser do tipo [menu]';
					return false;
				}
			}
		}
		return parent::alterar();
	}
	
	/**
	 * Verifica o menu antes de exclui-lo
	 *
	 * @return mixed
	 */
	public function excluir() {
		if($this->_conn->executaSQL($this->montaSELECT("COUNT(*) AS CONT","MENU_PAI={$_POST['MENU_ID']}"))!==false) {
			$_dados = $this->_conn->proximo();				
			if($_dados['CONT']>0) {
				$this->_msgextra = 'Menu possui submenus e n�o pode ser excluido';
				return false;
			}
		}
		return parent::excluir();
	}
	
	
	/**
	 * Rotina para gerar o c�digo HTML para menus
	 * Respeitando a estrutura drop-down
	 *
	 */
	public function geraMenuHTML($_permissoes,&$_autorizados) {
		if($this->_conn->executaSQL(($_strSQL=$this->montaSELECT("*","MENU_PAI=-1","MENU_ID")))!==false&&
		   $this->_conn->getNumRows()>0) {
			// Montar as arvore inicial
			$_tab = new tag(new tipotag("TABLE"),
							Array(new atributo("BORDER",0),
								  new atributo("CELLPADDING",0),
								  new atributo("CELLSPACING",0)));
			$_menu = Array();
			while(($_dados=$this->_conn->proximo())!==false) {
				$_menu[] = Array($_dados['MENU_ID'],$_dados['MENU_DESC'],$_dados['MENU_PROG']);
			}
			$_tab->addSubTag(new tag(new tipotag("TR")));
			foreach($_menu as $_item) {
				$_tab->getLastSubTag()->addSubTag(new tag(new tipotag("TD")));
				$_tab->getLastSubTag()->getLastSubTag()->addSubTag(new tag(
								new tipotag("UL"),
								Array(new atributo("ID","nivel1"))));
				$_tab->getLastSubTag()->getLastSubTag()->getLastSubTag()->addSubTag(new tag(
								new tipotag("LI"),
								Array(new atributo("CLASS","principal"))));
				$_tab->getLastSubTag()->getLastSubTag()->getLastSubTag()->getLastSubTag()->addSubTag(new tag(
								new tipotag("A"),
								Array(new atributo("HREF","javascript:void(0);"),
									  new atributo("ONCLICK",($_item[2]=='[menu]'||$_permissoes[$_item[0]]=='NEG'
									  			? "void(0);" 
									  			: "objdivFiltro.reset();objRelatorio.objme.reset();ObjProcAjax.run('{$_item[2]}','CORPO');"))),$_item[1]));
				if($_permissoes[$_item[0]]=='NEG') {
					$_tab->getLastSubTag()->getLastSubTag()->getLastSubTag()->getLastSubTag()->getLastSubTag()->AddAtributo(new atributo("STYLE","color:#a0a0a0;"));
				} elseif($_item[2]!='[menu]') {
					$_autorizados[] = $_item[2];
				}
				if($_item[2]=='[menu]') {
					// Buscar Todos os Filhos
					if(($_filhos=$this->geraListadeFilhos($_item[0],$_permissoes,$_autorizados))!==false) {
						$_tab->getLastSubTag()->getLastSubTag()->getLastSubTag()->
						getLastSubTag()->addSubTag($_filhos);
					}
				}
			}
			return $_tab->toHTML(false);
		} else {
			return "- Nenhum Menu Implementado -";
		}
	}
	
	/**
	 * Retorna os Filhos, netos, etc.. de um menu
	 *
	 * @param integer $_pai C�digo do menu raiz 
	 * @return tag ou false
	 */
	protected function geraListadeFilhos($_pai,$_permissoes,&$_autorizados) {
		if($this->_conn->executaSQL($this->montaSELECT("*","MENU_PAI={$_pai}","MENU_ID"))!==false&&
		   $this->_conn->getNumRows()>0) {
		   	$_filhos = new tag(new tipotag("UL"),Array(new atributo("ID","nivel2")));
			$_menu = Array();
			while(($_dados=$this->_conn->proximo())!==false) {
				$_menu[] = Array($_dados['MENU_ID'],$_dados['MENU_DESC'],$_dados['MENU_PROG'],$_dados['MENU_ICON']);
			}
		   	foreach($_menu as $_item) {
		   		$_att = Array();
		   		if($_item[2]=='[menu]') {
		   			$_att[] = new atributo("ID","sub");
		   			$_att[] = new atributo("CLASS","principal");
		   		} else {
		   			$_att[] = new atributo("ID","nosub");
		   		}
		   		$_filhos->addSubTag(new tag(new tipotag("LI"),$_att));
				$_filhos->getLastSubTag()->addSubTag(new tag(
									new tipotag("A"),
									Array(new atributo("HREF","javascript:void(0);"),
								  		  new atributo("ONCLICK",($_item[2]=='[menu]'||$_permissoes[$_item[0]]=='NEG'
									  			? "void(0);" 
									  			: "objdivFiltro.reset();objRelatorio.objme.reset();ObjProcAjax.run('{$_item[2]}','CORPO');")))));
				if($_permissoes[$_item[0]]=='NEG') {
					$_filhos->getLastSubTag()->getLastSubTag()->AddAtributo(new atributo("STYLE","color:#a0a0a0;"));
				} elseif($_item[2]!='[menu]') {
					$_autorizados[] = $_item[2];
				}
				if($_item[3]!=""&&$_item[3]!==null) {
					$_filhos->getLastSubTag()->getLastSubTag()->addSubTag(new tag(new tipotag("IMG",false),
									Array(new atributo("BORDER",0),
										  new atributo("WIDTH",20),
										  new atributo("ALIGN","ABSMIDDLE"),
										  new atributo("SRC","framework/imagens/{$_item[3]}"))));
				}
				$_filhos->getLastSubTag()->getLastSubTag()->addSubTag(new tag(new tipotag("SPAN"),
												Array(new atributo("ID",($_item[3]!=""&&$_item[3]!==null ? "img" : "noimg"))),
												$_item[1]));
				if($_item[2]=='[menu]') {
					// Buscar Todos os Filhos
					if(($_netos=$this->geraListadeFilhos($_item[0],$_permissoes,$_autorizados))!==false) {
						$_filhos->getLastSubTag()->addSubTag($_netos);
					}
				}
		   	}
			return $_filhos;
		} else {
			return false;
		}
	} 
	
	
	public function geraArvoreMenu($_pai=-1) {
		if($this->_conn->executaSQL(($_strSQL=$this->montaSELECT("*","MENU_PAI={$_pai}","MENU_ID")))!==false&&$this->_conn->getNumRows()>0) {
			$_menu = Array();
			while($this->proximo()!==false) {
				$_menu[] = Array($this->getCampo("MENU_ID")->getValor(),
								  $this->getCampo("MENU_DESC")->getValor(),
								  $this->getCampo("MENU_PROG")->getValor(),
								  $this->getCampo("MENU_ICON")->getValor());
			}
			foreach($_menu as $_k=>$_item) {
				if($_item[2]=='[menu]') {
					$_menu[$_k][] = $this->geraArvoreMenu($_item[0]);
				}
			}
		} else {
			return null;
		}
		return $_menu;
	}
	
	public function getListaMenus($_lista) {
		if($this->_conn->executaSQL(($_strSQL=$this->montaSELECT("*","MENU_PROG IN({$_lista})","MENU_PROG")))!==false&&$this->_conn->getNumRows()>0) {
			$_menu = Array();
			while($this->proximo()!==false) {
				$_menu[$this->getCampo("MENU_PROG")->getValor()] = Array($this->getCampo("MENU_DESC")->getValor(),
																		 $this->getCampo("MENU_ICON")->getValor());
			}
			return $_menu;
		} else {
			return Array();
		}
	}
	
}
?>