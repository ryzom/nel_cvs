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
		widget=TextAreaWidget()
	),
	TextField('email',
		widget=TextAreaWidget()
	),
	TextField('login',
		widget=TextAreaWidget()
	),
	TextField('pseudo',
		widget=TextAreaWidget()
	),
	IntegerField('seat',
		default=1,
		widget=IntegerWidget()		
	),
	TextField('comment',
		widget=TextAreaWidget()		
	),
),)
participantSchema['id'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}
participantSchema['title'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}
participantSchema['description'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}
participantSchema['fullName'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}
participantSchema['email'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}
participantSchema['login'].widget.visible = { 'view': 'hidden', 'edit': 'hidden',}

class participant(BaseContent):
	""" Register to event """
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
								
