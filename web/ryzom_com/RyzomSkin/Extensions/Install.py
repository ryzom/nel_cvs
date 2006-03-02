from Products.Archetypes.public import listTypes
from Products.Archetypes.Extensions.utils import installTypes, install_subskin
from Products.RyzomSkin.config import PROJECTNAME, GLOBALS
from Products.ExternalMethod.ExternalMethod import manage_addExternalMethod
from StringIO import StringIO
from Products.CMFCore.utils import getToolByName

def install(self):
    out = StringIO()    
    installTypes(self, out, listTypes(PROJECTNAME), PROJECTNAME)
    install_subskin(self, out, GLOBALS)
    portal = getToolByName(self,'portal_url').getPortalObject()
    manage_addExternalMethod(portal,'get_servers_status','Get the servers status',PROJECTNAME + '.get_servers_status','get_servers_status')
    out.write("Successfully installed %s." % PROJECTNAME)
    return out.getvalue()
