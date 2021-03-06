<?php
/**
 * Classe para gerenciamento de usu�rios do sistema
 *
 */
class usuario extends BASE {
	private $_Chave = "PHPWS2009";
	private $_Texto = "WALACE.SOARES";
	private $_Iv    = "ED.ERICA"; 
	
	public function __construct(BancoDados $_conn) {
		parent::__construct($_conn);
		$this->_nome_tabela = 'usuario';
		$this->addCampo(new inteiro("USUARIO_ID","C�digo do Usu�rio",10,null,null,false,false,true,null,null,null,true));
		$this->addCampo(new string("USUARIO_NOME","Nome",60,null,null,true,true,true,5,null,null,false,$_conn));
		$this->addCampo(new string("USUARIO_LOGIN","Identifica��o(login)",20,null,null,true,true,true,5,null,null,false,$_conn));
		$this->addCampo(new string("USUARIO_SENHA","Senha",15,null,null,true,true,false,4,null,null,false,$_conn));
		$this->addCampo(new string("CONFIRMA_SENHA","Confirama��o de Senha",15,null,null,true,true,false,'[USUARIO_SENHA]',null,null,false,$_conn));
		$this->addCampo(new string("USUARIO_EMAIL","Endere�o de e-mail",120,null,null,true,true,false,null,null,null,false,$_conn));
		$this->addCampo(new inteiro("USUARIO_ATIVO","Ativo",1,null,null,true,true,false,null,null,null,false));
		$this->getCampo("USUARIO_SENHA")->setComportamento_form("password");
		$this->getCampo("CONFIRMA_SENHA")->setComportamento_form("password");
		$this->getCampo("USUARIO_ATIVO")->setComportamento_form("checkbox");
		$this->getCampo("USUARIO_ATIVO")->setValor_fixo(1);
		if($_GET['ACAO']=='INC') {
			$this->getCampo("USUARIO_ATIVO")->setMarcar(true);
		}
		$this->getCampo("USUARIO_EMAIL")->setObrigatorio(false);
		$this->getCampo("USUARIO_SENHA")->setFiltro(false);
		$this->getCampo("CONFIRMA_SENHA")->setFiltro(false);
		$this->getCampo("USUARIO_ATIVO")->setFiltro(false);
		$this->getCampo("USUARIO_SENHA")->setRelatorio(false);
		$this->getCampo("CONFIRMA_SENHA")->setRelatorio(false);
	}
	
	/**
	 * Codifica a senha informada
	 *
	 * @param string $_senha
	 * @return base64
	 */
	protected function codificasenha($_senha) {
		$_cipher = mcrypt_module_open(MCRYPT_BLOWFISH,'','cbc','');
		mcrypt_generic_init($_cipher, $this->_Chave, $this->_Iv);
		$_enc = mcrypt_generic($_cipher,$_senha);
		mcrypt_generic_deinit($_cipher);
		return base64_encode($_enc);
	}
	
	/**
	 * Recupera uma Senha codificada
	 *
	 * @param base64 $_senha
	 * @return string
	 */
	protected function restaurasenha($_senha) {
		$_cipher = mcrypt_module_open(MCRYPT_BLOWFISH,'','cbc','');
		mcrypt_generic_init($_cipher, $this->_Chave, $this->_Iv);
		$_snh = mdecrypt_generic($_cipher,base64_decode($_senha));
		mcrypt_generic_deinit($_cipher);		
		return str_replace(chr(0),"",$_snh);
	}
	
	/**
	 * Retorna o comando SQL para gera��o do 
	 * conjunto de dados resultante da busca AJAX
	 *
	 * @return string
	 */
	public function montaSQLAjax() {
		return "SELECT USUARIO_ID,USUARIO_NOME FROM {$this->_nome_tabela} WHERE " . 
			 	" USUARIO_ID=" . ((int) $_POST['valor']) . 
				" OR LOWER(USUARIO_NOME) LIKE '%" . strtolower($_POST['valor']) . "%' " . 
				"ORDER BY USUARIO_NOME";
	}
	
	/**
	 * Retorna o comando SQL necess�rio para
	 * a busca do pr�ximo c�digo
	 *
	 * @return string
	 */
	public function montaSQLProximoCodigo() {
		return "SELECT MAX(USUARIO_ID) AS CODIGO FROM {$this->_nome_tabela}";
	}
	
	/**
	 * Antes de Incluir um usu�rio novo precisamos
	 * Buscar o pr�ximo c�digo disponivel, retirar o campo CONFIRMA_SENHA da lista
	 * e codificar a senha
	 *
	 * @return mixed
	 */
	public function incluir() {
		if(($_cod=$this->getProximoCodigo(true))=='[ERRO]') {
			return false;
		} else {
			$this->getCampo("USUARIO_ID")->setValor($_cod);
			$this->getCampo("USUARIO_ID")->setIncluir(true);
			$this->getCampo("CONFIRMA_SENHA")->setIncluir(false);
			if($this->getCampo("USUARIO_ATIVO")->getValor()===null||$this->getCampo("USUARIO_ATIVO")->getValor()!=1) {
				$this->getCampo("USUARIO_ATIVO")->setValor(0);
			}
			$this->getCampo("USUARIO_SENHA")->setValor($this->codificasenha($this->getCampo("USUARIO_SENHA")->getValor()));
			return parent::incluir();
		}
	}
	
	/**
	 * Antes de alterar precisamos retirar o campo CONFIRMA_SENHA da lista
	 * e codificar a senha
	 * 
	 * @return mixed
	 */
	public function alterar() {
		$this->getCampo("CONFIRMA_SENHA")->setAlterar(false);
		if($this->getCampo("USUARIO_ATIVO")->getValor()===null||$this->getCampo("USUARIO_ATIVO")->getValor()!=1) {
			$this->getCampo("USUARIO_ATIVO")->setValor(0);
		}
		$this->getCampo("USUARIO_SENHA")->setValor($this->codificasenha($this->getCampo("USUARIO_SENHA")->getValor()));
		return parent::alterar();
	}
	
	/**
	 * Precisamos restaurar a senha antes de disponibiliza-la
	 *
	 * @param Array $_d
	 */
	public function preenche($_d) {
		parent::preenche($_d);
		if($this->getCampo("USUARIO_SENHA")->getValor()!=""&&$this->getCampo("USUARIO_SENHA")->getValor()!==null) {
			$this->getCampo("USUARIO_SENHA")->setValor($this->restaurasenha($this->getCampo("USUARIO_SENHA")->getValor()));
			if($_GET['ACAO']=='ALT') {
				$this->getCampo("CONFIRMA_SENHA")->setValor($this->getCampo("USUARIO_SENHA")->getValor());
			}
		}
	}
	
	
	public function RecuperaSenha($_usuario) {
		if($this->_conn->executaSQL($this->montaSELECT("*","USUARIO_LOGIN='" . $this->_conn->escapeString($_usuario) . 
				"' OR USUARIO_EMAIL='" . $this->_conn->escapeString($_usuario) . "'"))!==false) {
			if($this->_conn->getNumRows()==1) {
				$this->proximo();
				if($this->getCampo("USUARIO_EMAIL")->getValor()==""||$this->getCampo("USUARIO_EMAIL")->getValor()===null) {
					$_res = "<span style='color:red;'>O Usu�rio n�o possui E-mail cadastrado. Fale com o Administrador</span>";
				} elseif($this->getCampo("USUARIO_ATIVO")->getValor()==0) {
					$_res = "<span style='color:red;'>O Usu�rio est� inativo no sistema. Fale com o Administrador</span>";
				} else {	
					$_msg  = "Prezado(a) {$this->getCampo("USUARIO_NOME")->getValor()},<br><br>";
					$_msg .= "Conforme sua solicita��o, segue abaixo seus dados cadastrados no sistema.<br><br>";
					$_msg .= "Usu�rio: {$this->getCampo("USUARIO_LOGIN")->getValor()} <br>";
					$_msg .= "Senha: {$this->getCampo("USUARIO_SENHA")->getValor()} <br><br><br><br>";
					$_msg .= "Atenciosamente,<br><br>Equipe de Administra��o<br>{$_SERVER['HTTP_HOST']}";
					if(@mail($this->getCampo("USUARIO_EMAIL")->getValor(),"Recupera��o de Senha",$_msg,"Content-Type: text/html\n")!==true) {
						$_res = "<span style='color:red;'>Ocorreu um erro ao Enviar o E-mail. Fale com o Administrador</span>";
					} else {
						$_res = "Dentro de instantes voc� receber� em seu endere�o de E-mail os seus dados cadastrais. Obrigado";
					}
				}
		   	} else {
		   		$_res = "<span style='color:red;'>O Usu�rio ou E-mail informado n�o encontrado. Fale com o Administrador</span>";
		   	}
		} else {
			$_res = "<span style='color:red;'>Erro na Busca. Fale com o Administrador<span>";
		}
		echo utf8_encode($_res);
	}
	
	/**
	 * Autentica��o de usuario
	 *
	 * @param string  $_usr
	 * @param string  $_snh
	 * @return mixed
	 */
	public function login($_usr,$_snh) {
		$_res = true;
		if($this->_conn->executaSQL($this->montaSELECT("*","USUARIO_LOGIN='" . $this->_conn->escapeString($_usr) . "'"))!==false) {
			if($this->_conn->getNumRows()==1) {
				$this->proximo();
				if($this->getCampo("USUARIO_ATIVO")->getValor()==0) {
					$_res = "O Usu�rio est� inativo no sistema. Fale com o Administrador";
				} elseif($this->getCampo("USUARIO_SENHA")->getValor()!=$_snh) {
					$_res = "A senha n�o confere, favor digitar novamente.";
				}
		   	} else {
		   		$_res = "O Usu�rio informado n�o foi encontrado no sistema";
		   	}
		} else {
			$_res = "Erro na Busca do usu�rio. Fale com o Administrador";
		}
		return $_res;
	}


	/**
	 * Retorna uma lista de usu�rios
	 *
	 * @return array
	 */
	public function listaUsuarios() {
		$_lista = Array();
		if($this->Buscar()!==false) {
			while($this->proximo()!==false) {
				$_lista[] = Array('valor'=>$this->getCampo('USUARIO_ID')->getValor(),
								  'label'=>$this->getCampo('USUARIO_NOME')->getValor());
			}
		}
		return $_lista;
	}
	
}
?>