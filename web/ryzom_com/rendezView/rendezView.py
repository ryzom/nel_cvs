# -*- coding: utf-8 -*-
from Products.CMFCore import CMFCorePermissions
try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *

from config import *

rendezViewSchema=BaseFolderSchema.copy()+ Schema((
	TextField('text',
		searchable=True,
		widget=RichWidget(description="Description de l'evenement",)
	),
	TextField('adress',		
		widget=TextAreaWidget(description="lieu de l'evenement",)
	),
	ImageField('plan',
		widget=ImageWidget(description="Inserer un plan",)
	),
	DateTimeField('date',
		required=True,
		widget=CalendarWidget(description="date de l'evenement",label="Date",)
	),
	DateTimeField('dateLimit',
		widget=CalendarWidget(description="date limite d'inscription",label="Date",)
	),
	IntegerField('nbSeat',
		widget=IntegerWidget(description="Nombre de place disponible",)
	),
#	IntergerField('maxSeat',
#		widget=IntegerWidget(description="Nombre de place rÃ©servable au maximum",default=1,)
#	),
#	LinesField('participant',		
#		widget=LinesWidget(),
#		visible={'edit':'hidden', 'view':'visible'},
#	),
),)
 
 
   
class rendezView(BaseFolder):

	"""Add an rendezView event"""
	schema = rendezViewSchema

	archetype_name = "rendezView"
	meta_type = 'rendezView'
	default_view  = 'rendezView_view'
	immediate_view = 'rendezView_view'
	allowed_content_types = ('participant',)
	actions = (
#		{
#		'id': 'view',
#		'name': 'view',
#		'action': 'string:${object_url}/rendezView_view',
#		'permissions': (CMFCorePermissions.View,)
#		},
#		{
#		'id': 'listing',
#		'name': 'listing',
#		'action': 'string:${object_url}/rendezView_listing',
#		'permissions': (CMFCorePermissions.View,)
#		},
	)

	def setTitle(self, value, **kwargs):
		self.getField('title').set(self, value, **kwargs)
		if value:
			try:
				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
			except:
				pass #try to do better than this


registerType(rendezView,PROJECTNAME)								
								
