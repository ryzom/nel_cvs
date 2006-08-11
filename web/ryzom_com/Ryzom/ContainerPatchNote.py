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

	actions = (
		{ 'id': 'view',
		'name': 'View',
		'action': 'string:${object_url}/ContainerPatchNote_view',
		'permissions': (CMFCorePermissions.View,)
		},
	)

	def setTitle(self, value, **kwargs):
		if not value and self.id:
			value = self.id
		else:
			value = re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower()
			try:
				self.setId(value)
			except:
				pass
		self.getField('title').set(self, value, **kwargs)

registerType(ContainerPatchNote, PROJECTNAME)
