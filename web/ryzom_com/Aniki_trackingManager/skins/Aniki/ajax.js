/*Script permettant d'ajaxifier le produit Aniki 'a tracking manager'*/

function changeContentOf(idDom, newContent){
	element = document.getElementById(idDom);
	/*element.style = "";*/
	element.innerHTML = newContent;
}

function hideOrNot(idElement){
	if(!idElement) return false;
	
	var toHide = document.getElementById(idElement);
	
	if(toHide.className=='hidden'){
		toHide.className='notHidden';
	}
	else{
		toHide.className='hidden';
	}
}

function hide(idElement){
	if(!idElement) return false;
	var toHide = document.getElementById(idElement);
	toHide.className='hidden';
}

function unHide(idElement){
	if(!idElement) return false;
	var toHide = document.getElementById(idElement);
	toHide.className='notHidden';
}

function changeContentURL(idDom){
	if (xmlhttp.readyState == 4){
		if (xmlhttp.status == 200){
			changeContentOf(idDom, xmlhttp.responseText);
		}
	}
}

function myXmlHttp(id, xmlHttp){
	this.xmlhttp = xmlHttp;
	this.func = new Function('if (this.xmlhttp.readyState == 4){ if (this.xmlhttp.status == 200){ changeContentOf('+id+', this.xmlhttp.responseText);} }');
}

function getHTTPObject(){
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
	

	return xmlhttp;
}

/*Layout d'une fonction de callback classique*/
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
	if (httpTruc){
		unHide(idDom);
		httpTruc.onreadystatechange = function(){
			changeContentOf(idDom, "chargement...");
			if (httpTruc.readyState == 4){ 
				if (httpTruc.status == 200){
					changeContentOf(idDom, httpTruc.responseText);
				} 
			}
		}
		try {
			httpTruc.open("GET", url, true);
		}
		catch(e){
			changeContentOf(idDom, 'erreur de chargement !!!');
			return false;
		}
		
		
		httpTruc.send(null);
	}
}