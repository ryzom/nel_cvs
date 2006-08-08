# -*- coding: utf-8 -*-
from Products.CMFCore.utils import getToolByName
from Products.CMFCore import CMFCorePermissions
try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *

from config import *

#dÃ©fini le schÃ©ma 
participantSchema=BaseSchema.copy()+ Schema((
	TextField('fullName',
		widget=TextAreaWidget(visible={'edit':'hidden', 'view':'hidden'},)		
	),
	TextField('email',
		widget=TextAreaWidget(visible={'edit':'hidden', 'view':'hidden'},)		
	),
	TextField('login',
		widget=TextAreaWidget(visible={'edit':'hidden', 'view':'hidden'},)		
	),
	IntegerField('seat',
		default=1,
		widget=IntegerWidget()		
	),
),)
participantSchema['id'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}
participantSchema['title'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}
participantSchema['description'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}

class participant(BaseContent):
	
	schema = participantSchema

	archetype_name = "participant"
	meta_type = 'participant'
	global_allow = 0

#	def setTitle(self):
#		value = self.getLogin()+'_'+self.getFullName()
#		self.getField('title').set(self, value, **kwargs)

#	def setFullName(self):
#		mtool = getToolByName(self, 'portal_membership')
#		member = mtool.getAuthenticatedMember()
#		value = member.getProperty('fullname')
#		self.getField('fullName').set(self, value, **kwargs)

#	def setEmail(self):
#		mtool = getToolByName(self, 'portal_membership')
#		member = mtool.getAuthenticatedMember()
#		value = member.getProperty('email')
#		self.getField('email').set(self, value, **kwargs)

#	def setLogin(self):
#		mtool = getToolByName(self, 'portal_membership')
#		member = mtool.getAuthenticatedMember()
#		value = member.getUserName()
#		self.getField('login').set(self, value, **kwargs)
	

registerType(participant,PROJECTNAME)								
								
