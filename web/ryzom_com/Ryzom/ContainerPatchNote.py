# -*- coding: utf-8 -*-
try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *
from Products.CMFCore import CMFCorePermissions
from config import PROJECTNAME

ContainerPatchNoteSchema = BaseFolderSchema.copy() + Schema((),)

class ContainerPatchNote(BaseFolder):
	"""Add a Container for Patch Note"""
	schema = ContainerPatchNoteSchema
	meta_type = portal_type = 'ContainerPatchNote'
	allowed_content_types = ('UrlPatchNote',)

	#actions = (
	#	{ 'id': 'view',
	#	'name': 'View',
	#	'action': 'string:${object_url}/ContainerPatchNote_view',
	#	'permissions': (CMFCorePermissions.View,)
	#	},
	#)

registerType(ContainerPatchNote, PROJECTNAME)
