from Products.Archetypes.public import process_types, listTypes

# CMF imports
from Products.CMFCore import utils
from Products.CMFCore.CMFCorePermissions import setDefaultRoles,AddPortalContent
from Products.CMFCore.DirectoryView import registerDirectory
from config import *

AddPatchNote = 'Add new Event - rendezView'
setDefaultRoles( AddRendezView, ( 'Manager', 'Reviewer' ) )


registerDirectory(SKINS_DIR, GLOBALS)

def initialize(context):
	import rendezView
	import participant

	content_types, constructors, ftis = process_types(
		listTypes(PROJECTNAME),
		PROJECTNAME)
	
	allTypes = zip(content_types, constructors)
	for atype, constructor in allTypes:		
		kind = "%s: %s" % (config.PROJECTNAME, atype.meta_type)
		utils.ContentInit(
		kind,
		content_types      = (atype,),
 		permission         = AddPatchNote,
 		extra_constructors = (constructor,),
 		fti                = ftis,
		).initialize(context)

