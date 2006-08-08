# -*- coding: utf-8 -*-
from Products.CMFCore.utils import getToolByName
from Products.CMFCore import CMFCorePermissions
from AccessControl import ClassSecurityInfo
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
	IntegerField('maxSeat',
		default=1,
		widget=IntegerWidget(description="Nombre de place rÃ©servable au maximum par personne",)
	),
#	LinesField('participant',		
#		widget=LinesWidget(),
#		visible={'edit':'hidden', 'view':'visible'},
#	),
),)
 
 
   
class rendezView(BaseFolder):

	"""Add an rendezView event"""
	schema = rendezViewSchema
	security = ClassSecurityInfo()
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
		"""remplace l'id par un identificateur plus conviviale"""
		self.getField('title').set(self, value, **kwargs)
		if value:
			try:
				self.setId(re.sub('[^A-Za-z0-9_-]', '', re.sub(' ', '-', value)).lower())
			except:
				pass #try to do better than this

	#test si un membres peut s'inscrire, (s'il reste de la place et s'il n'est pas dÃ©ja inscrit)
	#dÃ©ja inscrit ? on testera dans la vue
	def isAvailable(self):
		"""test si des places sont disponible"""
		#on recupere le membre
		#mtool = getToolByName(self, 'portal_membership')
		#member = mtool.getAuthenticatedMember()
		#id = member.getUserName()+'_'+member.getProperty('fullname')
		#recuperer la liste des participants de l'event, et le nombre de place reserver
		s = 0
		tab = self.getParticipant()		
		for i in tab:
			o = i.getObject()
			s += o.getSeat()
		return s < self.getNbSeat()


	def nbParticipant(self):
		"""renvoie le nombre d'inscriptions"""		
		tab = self.getParticipant()
		return len(tab)
			
		
	def getParticipant(self):
		"""renvoie la listes des participants"""
		path = '/'.join(self.getPhysicalPath())
		results = self.portal_catalog(meta_type=['participant',],path={'query':path, 'level': 0},)
		return results

	def addParticipant(self,seat=1):
		"""ajoute une inscription"""
		path = '/'.join(self.getPhysicalPath())
		mtool = getToolByName(self, 'portal_membership')
		member = mtool.getAuthenticatedMember()
		fullname = member.getProperty('fullname')
		login = member.getUserName()
		email = member.getProperty('email')
		seat = seat

		newid = self.getId()+'_'+login+'_'+fullname
		title = login+'_'+fullname		
		self.invokeFactory(type_name='participant', id=newid, title=title, description='', fullName=fullname, email=email, seat=seat)
		new_obj = getattr(self, newid)
		new_obj.setFullName(fullname)
		new_obj.setLogin(login)
		new_obj.setEmail(email)
		new_obj.setSeat(seat)

		return "[inscription success]"
	
		


registerType(rendezView,PROJECTNAME)								
								
