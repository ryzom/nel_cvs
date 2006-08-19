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
		default_output_type='text/plain',
		allowable_content_types=(
			'text/restructured',
			'text/plain',
			'text/html',
		),
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

	actions = (
		{
		'id': 'edit',
		'name': 'Edit',
		'action': 'string:${object_url}/rendezView_register_edit',
		'permissions': (CMFCorePermissions.ModifyPortalContent,)
		},
	)

registerType(participant,PROJECTNAME)
								
