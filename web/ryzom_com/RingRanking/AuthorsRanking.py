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
))

class AuthorsRanking(BaseContent):
	"""Don't **use** this object"""
	security = ClassSecurityInfo()
	schema = AuthorsRankingSchema
	meta_type = portal_type = 'AuthorsRanking'
	global_allow = 0
	_at_rename_after_creation = True

#	actions = (
#		{ 'id': 'view',
#		'name': 'View',
#		'action': 'string:${object_url}/AuthorsRanking_view',
#		'permissions': (CMFCorePermissions.View,)
#		},
#	)


	## {rang : [info sur le scenario]}
	Ranking={}
	security.declareProtected(CMFCorePermissions.View, 'getRanking')
	def getRanking(self):
		"""return the ranking's list"""
		return self.Ranking
	
#	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'updateRanking')
#	def updateRanking(self,d):
#		"""update the ranking's list"""
#		self.Ranking.update(d)

	## stocker le rÃ©sultat de la requete SQL
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'update')
	def update(self):
		"""update Ranking"""

		if self.getAM():
			ranking_by = 'rrp_am'
		else:
			ranking_by = 'rrp_author'		
		## SQL Request
		try:
			request = self.SQL_AuthorsRanking(ranking_by=ranking_by)
		except:
			return 'Ranking Update Failed'
		
		## Format Result of the request
		formatted_request=self.FormatRequest(request)
		## store Result formatted
		self.Ranking = formatted_request
		return 'AuthorsRanking Update Success'

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'FormatRequest')
	def FormatRequest(request):
		"""retourne un dictionnaire avec le rang comme clef"""
		result = {}
		rank = 0
		for row in request:
			rank+=1
			#get the guild name
			try:
				guild = self.SQL_GuildName(guild_id=row[3])
			except:
				guild = ''

			#get if charecters's users is pioneer
			try:
				request = self.SQL_GetPrivileges(user_id=row[2])				
			except:
				request = 'no stats'
			if 'PIONEER' in request:
				pioneer = 1
			else:
				pioneer = 0
			
			#create information
			info = {'rank':rank,
				'name':row[0],
				'guild':guild,
				'pioneer':pioneer,
				'score_am':row[3],
				'score_author':row[3],
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
