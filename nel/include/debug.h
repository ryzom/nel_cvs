/*
 *  debug.h : all debug tools
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

#include "types.h"

/*
 * stop :	Quitte l'application car une erreur s'est produite. A utiliser par exemple dans le cas d'un switch 'default'
 *
 * ex:
 * switch (t) {
 *  case 1: ... break;
 *  case 4: ... break;
 *  default: stop;	// should never happen
 * }
 */

#ifdef RELEASE
#define stop
#else
#define stop assert (false)
#endif

/*
 * assert (exp) :	Fonction qui permet de s'assurer de la veracite de 'exp'. A utiliser pour etre sur qu une variable
 *					a bien un contenu valide. Ne fait rien c est 'exp' est vrai et quitte l'application dans le cas contraire.
 *
 * ex: assert (Mesh!=NULL);
 */

#ifdef RELEASE
#define assert(_exp_)
#else
inline void assert (bool expr)
{
	if (!expr)
	{
#ifdef DEBUG

#ifdef OS_WINDOWS
		// mets un break point sous windows
		__asm int 3
#else
		// affiche un message d'erreur a l'ecran et quitte l'application
#endif

#elif BETA
		// envoie un mail et quitte, affiche un message d'erreur a l'ecran et quitte l'application
#endif
	}
}
#endif


/*
 * assert2 (exp, r) :	Meme fonction que assert mais affiche le message passe en parametre.
 *						WARNING: La fonction assert2 doit tenir sur une ligne.
 *
 * ex: assert2 (Type<12, "Bad Type (T=%d)", Type);
 */

#ifdef _RELEASE
#define assert2 //
#else
inline void assert2 (bool expr, const char *reason, ...)
{
	if (!expr)
	{
#ifdef _DEBUG

#ifdef OS_WINDOWS
		__asm int 3
#else
		// affiche un message d'erreur avec 'reason' a l'ecran et quitte l'application
#endif

#elif _BETA
		// envoie un mail avec 'reason', affiche un message d'erreur avec 'reason' a l'ecran et quitte l'application
#endif
	}
}
#endif

// TODO: voir si un verify est vraiment indispensable

#endif // DEBUG_H
