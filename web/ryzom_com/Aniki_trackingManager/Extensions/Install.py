from Products.ExternalMethod.ExternalMethod import manage_addExternalMethod
from Products.CMFCore.utils import getToolByName

from Products.Archetypes.public import listTypes
from Products.Archetypes.Extensions.utils import installTypes, install_subskin
from Products.Aniki.config import PROJECTNAME, GLOBALS
from StringIO import StringIO


def install(self,reinstall=False):

	out = StringIO()    
	installTypes(self, out, listTypes(PROJECTNAME), PROJECTNAME)
	install_subskin(self, out, GLOBALS)

	#print>>out("Successfully installed %s." % PROJECTNAME)
	return out.getvalue()

