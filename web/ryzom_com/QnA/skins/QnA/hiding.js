//prend en parametre l'id de l'objet a cacher/decouvrir
//resultat: cache/decouvre l'objet au meme niveau dont la classe est hideable
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
	
/*
@param: idcontainer, the of the container where to search 
*/
function hideAll(idContainer){
	var iterator = 0;
	var a = document.getElementById(idContainer);
	var spanz = a.getElementsByTagName('span');
	while(iterator < spanz.lenght){
		if(spanz[iterator].className == "notHidden"){
			spanz[iterator].className = "hidden";
		}
	}
}

window.onload = hideAll("edit_form");
