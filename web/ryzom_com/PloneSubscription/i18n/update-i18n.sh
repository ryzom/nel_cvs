#!/bin/sh

echo "usage: $0 domain potfile"
echo "exemple: $0 plone i18n/PloneSubscription-plone.pot"
echo $0 $1 $2

TEMPLATES=`find $PWD/skins -iregex '.*\..?pt$'`
I18NBIN=i18ndude

#echo $I18NBIN find-untranslated $TEMPLATES
#$I18NBIN find-untranslated $TEMPLATES

# i18ndude find-untranslated `find skins -iregex '.*\..?pt$'`
# i18ndude rebuild-pot --pot plonesubscription.pot --create plonesubscription --merge i18n/PloneSubscription.pot skins/
# i18ndude rebuild-pot --pot plone.pot --create plone --merge i18n/PloneSubscription-plone.pot skins/
# i18ndude filter plone.pot ../CMFPlone/i18n/plone.pot > i18n/PloneSubscription-plone.pot

echo "$I18NBIN rebuild-pot --pot $1.pot --create $1 --merge $2 skins"
$I18NBIN rebuild-pot --pot $1.pot --create $1 --merge $2 skins


echo msgmerge -U $2-plone_fr.po $2-plone.pot
