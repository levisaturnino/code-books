<?php
/**
 * Classe para Auditoria do Sistema
 *
 */
class auditoria extends BASE {
	
	public function __construct(BancoDados $_conn) {
		parent::__construct($_conn);
		$this->_nome_tabela = 'auditoria';
		$this->addCampo(new inteiro("AUDITORIA_ID","C�digo do Hist�rico",10,null,null,true,false,false,null,null,null,false));
		$this->addCampo(new string("USUARIO_LOGIN","Usu�rio",20,null,null,true,false,true,null,null,null,true,$_conn));
		$this->addCampo(new string("IP","IP",60,null,null,true,false,true,null,null,null,false,$_conn));
		$this->addCampo(new string("CLASSE","Classe",60,null,null,true,false,true,null,null,null,false,$_conn));
		$this->addCampo(new datahora("DATA","Data/Hora",20,null,null,true,false,true,null,null,null,false,$_conn));
		$this->addCampo(new string("FUNCAO","Fun��o",3,null,null,true,false,true,null,null,null,false,$_conn));
		$this->addCampo(new string("HISTORICO","Hist�rico",2000,null,null,true,false,true,null,null,null,false,$_conn));
		$this->getCampo("HISTORICO")->setComportamento_form("textarea");
		$this->_exibe_opcoes = false;
		$this->getCampo("HISTORICO")->setFiltro(false);
	}
	
	/**
	 * Retorna o comando SQL necess�rio para
	 * a busca do pr�ximo c�digo
	 *
	 * @return string
	 */
	public function montaSQLProximoCodigo() {
		return "SELECT MAX(AUDITORIA_ID) AS CODIGO FROM {$this->_nome_tabela}";
	}
	
	public function getOrdem() {
		return "DATA DESC,USUARIO_LOGIN";
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
			$this->getCampo("AUDITORIA_ID")->setValor($_cod);
			$this->getCampo("USUARIO_LOGIN")->setValor($_SESSION['USUARIO_LOGIN']);
			$this->getCampo("IP")->setValor($_SERVER['REMOTE_ADDR']);
			$this->getCampo("DATA")->setValor(date("d-m-Y H:i:s"));
			return parent::incluir();
		}
	}
	
	/**
	 * N�o � possivel alterar um hist�rico
	 */
	final public function alterar() {
		return false;
	}
	
	/**
	 * N�o � possivel excluir um hist�rico
	 *
	 */
	final public function excluir() {
		return false;
	}
	
	/**
	 * N�o devemos exibir o formul�rio
	 *
	 */
	final public function getFormulario($_fcn="INC") {
		return "A classe n�o suporta esta funcionalidade";
	}
	
	/**
	 * O campo HISTORICO deve ser particionado antes
	 * de sua exibi��o na lista
	 */
	public function preenche($_d) {
		parent::preenche($_d);
		$_span = new tag(new tipotag("SPAN"),
					Array(new atributo("STYLE","font-size:11px;color:navy")),
					wordwrap($this->getCampo("HISTORICO")->getValor(),60,"<br>",true));
		$this->getCampo("HISTORICO")->setValor($_span->toHTML());
	}
	

}
?>