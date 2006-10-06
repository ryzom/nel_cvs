/* Scripts permettant d'ajaxifier le produit Aniki 'a tracking manager' 
   ici adapté pour les besoins de Ring Ranking
   
   TODO: wrapper toutes ces fonctions dans un seul et meme objet de 
   maniere a pouvoir mettre facilement des fonctions de callback 
   personalisés => cherche un moyen de ne pas passer pas des fonctions 
   anonymes pour la fonction de callback*/


/* Non liée à AJAX, cette fonction remplace le contenu d'un element dont
   l'identifiant est idDom par newContent */
function changeContentOf(idDom, newContent){
	element = document.getElementById(idDom);
	element.innerHTML = newContent;
}

/* Non liée à AJAX, cette fonction permet de caché ou decaché un 
   élément dont l'id est idElement suivant si il est, respectivement, 
   décaché ou caché
   Retourne faux si l'element n'existe pas*/
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

/* Non liée à AJAX, cette simple fonction cache un element dont l'id 
   est idElement
   Retourne faux si l'élément n'existe pas. */
function hide(idElement){
	if(!idElement) return false;
	var toHide = document.getElementById(idElement);
	toHide.className='hidden';
}

/* Non liée à AJAX, cette simple fonction décache un element dont l'id 
   est idElement
   Retourne faux si l'élément n'existe pas. */
function unHide(idElement){
	if(!idElement) return false;
	var toHide = document.getElementById(idElement);
	toHide.className='notHidden';
}

/* Essaie de clarification... 
   ne marche pas actuellement*/
function myXmlHttp(id, xmlHttp){
	this.xmlhttp = xmlHttp;
	this.func = new Function('if (this.xmlhttp.readyState == 4){ if (this.xmlhttp.status == 200){ changeContentOf('+id+', this.xmlhttp.responseText);} }');
}


/* Fonction permettant de recupérer un objet XMLhttpRequest quelquesoit 
   le navigateur.
*/
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

/* Layout d'une fonction de callback classique
   Cette fonction est appelée a chaque changement d'etat
   Quand l'état de la requète est 4 (contenu reçu) alors elle affiche
   le contenu dans une une boite de dialogue "alert" */
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

/* Fonction qui envoie une requete, url, et qui remplace le contenu de 
   idDom par la reponse a la requete*/
function sendRequestAndChangeContent(idDom, url){
	var httpTruc = getHTTPObject();
	if (httpTruc){  //Si notre objet est bien créé alors on continue
		unHide(idDom);
		httpTruc.onreadystatechange = 
		function(){ // C'est mal les fonctions anonymes!
			changeContentOf(idDom, "chargement..."); //Par défaut, on affiche un message indiquant le chargement
			if (httpTruc.readyState == 4){ // cf fonction canevas ci dessus
				if (httpTruc.status == 200){
					changeContentOf(idDom, httpTruc.responseText); //On change le contenu de idDom
				} 
				else {
					changeContentOf(idDom, "erreur");
				}
			}
		}
		try {
			httpTruc.open("GET", url, true); //On envoie la requete
		}
		catch(e){
			changeContentOf(idDom, 'erreur de chargement !!!'); 
			return false;
		}
		
		
		httpTruc.send(null);
	}
	else {
		alert('Javascript error: your browser is too old or too dumb'+
				' or both...');
	}
}