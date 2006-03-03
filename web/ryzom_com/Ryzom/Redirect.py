try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *
from Products.CMFCore import CMFCorePermissions
from config import PROJECTNAME
import re

RedirectSchema = BaseSchema.copy() + Schema((
	StringField('remote_url',
	        required=True,
        	searchable=True,
	        default = "http://",
        	validators = ('isURL',),
        	widget = StringWidget(label = "URL",)
	),
    ),)


class Redirect(BaseContent):
	schema=RedirectSchema
	actions = (
		{ 'id': 'view',
		'name': 'View',
		'action': 'string:${object_url}/redirect_view',
		'permissions': (CMFCorePermissions.View,)
		},
	)
	def setTitle(self, value, **kwargs):
		self.getField('title').set(self, value, **kwargs)
		if value:
			weblog = self.quills_tool.getParentWeblog(self)
			self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())


registerType(Redirect,PROJECTNAME)
