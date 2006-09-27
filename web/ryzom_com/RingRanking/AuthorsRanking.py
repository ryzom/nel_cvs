# -*- coding: utf-8 -*-

#import zope/archetype
from AccessControl import ClassSecurityInfo
from Products.CMFCore.utils import getToolByName
from Products.CMFCore import CMFCorePermissions

try:
    from Products.LinguaPlone.public import *
except ImportError: 
    from Products.Archetypes.public import *

#import de fonction du produit
from config import *

AuthorsRankingSchema=BaseSchema.copy()+ Schema((
	BooleanField('AM',
		default = False,
		widget=BooleanWidget(
			description="Select for Animator Ranking **not use for the moment**"
		),
	),
	LinesField('lang',
		required=True,
		vocabulary=['en','fr','de'],
		widget=SelectionWidget(
			description="Choose a language",
		),
	),
))

class AuthorsRanking(BaseContent):
	"""Don't **use** this object"""
	security = ClassSecurityInfo()
	schema = AuthorsRankingSchema
	meta_type = portal_type = 'AuthorsRanking'
	global_allow = 0
	_at_rename_after_creation = True

	actions = (
		{ 'id': 'view',
		'name': 'View',
		'action': 'string:${object_url}/AuthorRanking_view',
		'permissions': (CMFCorePermissions.View,)
		},
	)


	## {rang : [info sur le scenario]}
	Ranking={}
	security.declareProtected(CMFCorePermissions.View, 'getRanking')
	def getRanking(self):
		"""return the ranking's list"""
		return self.Ranking
	
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'setRanking')
	def setRanking(self,d):
		"""set the ranking's list"""
		self.Ranking = d

	security.declareProtected(CMFCorePermissions.View, 'isAdventureMaster')
	def isAdventureMaster(self):
		"""return if the rank is for Adventure Master"""
		return self.getAM()

	## stocker le rÃ©sultat de la requete SQL
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'update')
	def update(self):
		"""update Ranking"""
		lng=self.getLang()
		lang = 'lang_en'
		if 'fr' in lng:
			lang = 'lang_fr'
		elif 'de' in lng:
			lang = 'lang_de'

		if self.getAM():
			ranking_by = 'rrp_am'
		else:
			ranking_by = 'rrp_author'		
		## SQL Request		
		try:
			request = self.zsql.SQL_AuthorsRanking(ranking_by=ranking_by,language=lang)
		except:
			return 'Ranking Update Failed'
		
		## Format Result of the request
		formatted_request=self.FormatRequest(request)
		## store Result formatted
		self.setRanking(formatted_request)
		return 'AuthorsRanking Update Success'

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'FormatRequest')
	def FormatRequest(self,request):
		"""retourne un dictionnaire avec le rang comme clef"""
		result = {}
		rank = 0
		for row in request:
			rank+=1
			guild = ''
			pioneer = 0
			#get the guild name
			try:
				#this SQL return one row of one column
				guild = self.zsql.SQL_GuildName(guild_id=row[3])[0][0]
			except:
				guild = ''

			#get if characters's users is pioneer			
			try:
				#this SQL return one row of one column
				pioneer = self.zsql.SQL_GetPrivileges(user_id=row[2])[0][0]			
			except:
				pioneer = ''
			if 'PIONEER' in pioneer:
				pioneer = 'Pioneer'
			
			#create information
			info = {'rank':rank,
				'name':row[1],
				'guild':guild,
				'pioneer':pioneer,
				'score_am':row[4],
				'score_author':row[6],
				'language':row[7],
				}
			#update dictionnarie
			result.update({rank:info})
		return result


#	security.declareProtected(CMFCorePermissions.View, 'sortedRanking')
#	def sortedRanking(ranking_by=none):
#		"""return a sorted ranking tab"""
#		ranking = self.getRanking()
#		return ranking

		

registerType(AuthorsRanking,PROJECTNAME)
