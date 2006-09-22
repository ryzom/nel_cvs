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

OwnersRankingSchema=BaseSchema.copy()+ Schema((
	BooleanField('Masterless',
		default = False,
		widget=BooleanWidget(
			description="Select for Masterless Ranking"
		),
	),
))

class OwnersRanking(BaseContent):
	"""Don't **use** this object"""
	security = ClassSecurityInfo()
	schema = OwnersRankingSchema
	meta_type = portal_type = 'OwnersRanking'
	global_allow = 0
	_at_rename_after_creation = True

#	actions = (
#		{ 'id': 'view',
#		'name': 'View',
#		'action': 'string:${object_url}/OwnersRanking_view',
#		'permissions': (CMFCorePermissions.View,)
#		},
#	)


	## {rang : [info sur le scenario]}
	Ranking={}
	security.declareProtected(CMFCorePermissions.View, 'getRanking')
	def getRanking(self):
		"""return the ranking's list"""
		return self.Ranking
	
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'updateRanking')
	def updateRanking(self,d):
		"""update the ranking's list"""
		self.Ranking.update(d)

	## stocker le rÃ©sultat de la requete SQL
	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'update')
	def update(self):
		"""update Ranking"""		
		## SQL Request
		try:
			request = self.SQL_OwnersRanking(ranking_by='rrp_scored')
		except:
			return 'Ranking Update Failed'
		## Format Result of the request
		formatted_request=self.FormatRequest(request)
		## store Result formatted
		self.updateRanking(formatted_request)		
		return 'OwnersRanking Update Success'

	security.declareProtected(CMFCorePermissions.ModifyPortalContent, 'FormatRequest')
	def FormatRequest(request):
		"""retourne un dictionnaire avec le rang comme clef"""
		result = {}
		rank = 0		
		for row in request:
			rank+=1
			guild_name = self.SQL_GuildName(guild_id=str(row[2]))
			masterless = self.SQL_AnimMode(char_id=str(row[3]),anim_mode='am_autonomous')
			mastered = self.SQL_AnimMode(char_id=str(row[3]),anim_mode='am_dm')
			mastered_score = self.SQL_MasteredScore(char_id=str(row[3]))
			info = {'rank':rank,
				'name':row[0],
				'score':row[1],
				'mastered_score':mastered_score,
				'guild':guild_name,
				'mastered':mastered,
				'masterless':masterless,				
				}
			result.update({rank:info})
		return result


	security.declareProtected(CMFCorePermissions.View, 'sortedRanking')
	def sortedRanking(ranking_by='rank'):
		"""return a sorted ranking tab"""
		ranking = self.getRanking()
		return ranking

		

registerType(OwnersRanking,PROJECTNAME)
