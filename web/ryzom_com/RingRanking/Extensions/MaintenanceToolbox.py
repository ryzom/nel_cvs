def runUpdateRingRanking(self):
	"""Maintenance Task for RingRanking"""
	results = self.portal_catalog(meta_type='RingRanking')
	report = ''
	for brain in results:
		obj = brain.getObject()
		res = obj.updateAllRank()
		report+=str(obj.getId()) +'--'+ str(res)+'\n'
	return report
