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
		widget=RichWidget(
			label="",
			label_msgid="rendezView_schema_label_text",		
			description="Description de l'Ã©venement",
			description_msgid="rendezView_schema_text",
			i18n_domain="rendezView",
		)
	),
	TextField('address',
		widget=TextAreaWidget(
			label="",
			label_msgid="rendezView_schema_label_address",
			description="lieu de l'evenement",
			description_msgid="rendezView_schema_address",
			i18n_domain="rendezView",
		)
	),
	ImageField('plan',
		widget=ImageWidget(
			label="",
			label_msgid="rendezView_schema_label_plan",
			description="Inserer un plan",			
			description_msgid="rendezView_schema_plan",
			i18n_domain="rendezView",
		)
	),
	DateTimeField('date',
		required=True,
		widget=CalendarWidget(
			label="",
			label_msgid="rendezView_schema_label_date",
			description="date de l'evenement",			
			description_msgid="rendezView_schema_date",
			i18n_domain="rendezView",			
		)
	),
	DateTimeField('dateLimit',
		widget=CalendarWidget(
			label="",
			label_msgid="rendezView_schema_label_datelimit",
			description="date limite d'inscription",
			description_msgid="rendezView_schema_datelimit",
			i18n_domain="rendezView",
		)
	),
	IntegerField('nbSeat',
		default=1,
		widget=IntegerWidget(
			label="",
			label_msgid="rendezView_schema_label_nbSeat",
			description="Nombre de place disponible",
			description_msgid="rendezView_schema_nbseat",
			i18n_domain="rendezView",
		)
	),
	IntegerField('maxSeat',
		default=1,
		widget=IntegerWidget(
			label="",
			label_msgid="rendezView_schema_label_maxseat",
			description="Nombre de place rÃ©servable au maximum par personne",
			description_msgid="rendezView_schema_maxseat",
			i18n_domain="rendezView",
		)
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
#		{
#		'id': 'register',
#		'name': 'register',
#		'action': 'string:${object_url}/addParticipant',
#		'permission': (CMFCorePermissions.View,)
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
		mtool = getToolByName(self, 'portal_membership')
		inscriptId = self.getId()+'_'+mtool.getAuthenticatedMember().getUserName()
		path = '/'.join(self.getPhysicalPath())
		if self.portal_catalog(id=inscriptId,meta_type=['participant',],path={'query':path, 'level': 0},):
			return False

		return str(self.nbSeatsTake() < self.getNbSeat())

	def nbSeatsTake(self):
		"""renvoie les place prises"""
		s = 0
		tab = self.getParticipant()		
		for i in tab:
			o = i.getObject()
			s += o.getSeat()
		return s


	def nbParticipant(self):
		"""renvoie le nombre d'inscriptions"""		
		tab = self.getParticipant()
		return len(tab)
			
		
	def getParticipant(self):
		"""renvoie la listes des participants"""
		path = '/'.join(self.getPhysicalPath())
		results = self.portal_catalog(meta_type=['participant',],path={'query':path, 'level': 0},)
		return results


	security.declarePublic('addParticipant')
	def addParticipant(self,seat=1,comment=''):
		"""ajoute une inscription"""
		if not (self.isAvailable()):
			return "[Plus d'inscription disponible]"
		path = '/'.join(self.getPhysicalPath())
		mtool = getToolByName(self, 'portal_membership')
		member = mtool.getAuthenticatedMember()
		fullname = member.getProperty('fullname')
		login = member.getUserName()
		email = member.getProperty('email')
		seat = seat
		comment = comment
		newid = self.getId()+'_'+login
		title = login
		try:
			self.invokeFactory(type_name='participant', id=newid, title=title, description='', fullName=fullname, email=email, seat=seat)
			new_obj = getattr(self, newid)
			new_obj.setFullName(fullname)
			new_obj.setLogin(login)
			new_obj.setEmail(email)
			new_obj.setSeat(seat)
			new_obj.setComment(comment)
			return "[inscription success]"
		except:
			return "[inscription deja faite]"


	def nbSeatsRestant(self):
		"""retourne le nombre de places restantes"""
		return self.getNbSeat()-self.nbSeatsTake()

	def seats(self):
		"""retourne place prise / place totale"""
		return str(self.nbSeatsTake())+'/'+str(self.getNbSeat())


registerType(rendezView,PROJECTNAME)
