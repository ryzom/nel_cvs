def runUpdateRingRanking(self):
	"""Maintenance Task for RingRanking"""
	results = self.portal_catalog(meta_type='RingRanking')
	report = []
	for brain in results:
		obj = brain.getObject()
		res = obj.updateAllRank()
		report.append(str(obj.getId()) +'--'+ str(res))
	return report
