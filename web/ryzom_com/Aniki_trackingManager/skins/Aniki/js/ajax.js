var GLOBAL_ID_TO_MOD = '';

function MyHttpObject(id){
	this.callback = function changeContentURL(idDom){
	if (xmlhttp.readyState == 4){
		if (xmlhttp.status == 200){
			changeContentOf(GLOBAL_ID_TO_MOD, xmlhttp.responseText);
		}
	}
}

function changeContentOf(idDom, newContent){
	element = document.getElementById(idDom);
	element.innerHTML = newContent;
}
	this.id = id;
	

}



function getHTTPObject(func){
	var xmlhttp = false;
	
	/* Compilation conditionnelle d'IE */
	/*@cc_on
	@if (@_jscript_version >= 5)
		try{
			xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e){
			try{
				xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
			}
			catch (E){
				xmlhttp = false;
			}
		}
	@else
	
	xmlhttp = false;
	
	@end @*/
	
	/* on essaie de créer l'objet si ce n'est pas déjà fait */
	if (!xmlhttp && typeof XMLHttpRequest != 'undefined'){
		try{
			xmlhttp = new XMLHttpRequest();
		}
		catch (e){
			xmlhttp = false;
		}
	}
	
	if (xmlhttp){
		/* on définit ce qui doit se passer quand la page répondra */
		xmlhttp.onreadystatechange = func();
	}
	return xmlhttp;
}

function example(){
	/*Verifie si l'etat est bien 4 (fini)*/
	if (xmlhttp.readyState == 4){
		/* 200 : code HTTP pour OK */
		if (xmlhttp.status == 200){
			/*
			Traitement de la réponse.
			Ici on affiche la réponse dans une boîte de dialogue.
			*/
			alert(xmlhttp.responseText);
		}
	}
}



function sendRequestAndChangeContent(idDom, url){
	var httpTruc = getHTTPObject();
	
}