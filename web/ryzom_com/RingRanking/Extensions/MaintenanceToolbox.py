def runUpdateRingRanking(self):
	"""Maintenance Task for RingRanking"""
	results = self.portal_catalog(meta_type='RingRanking')
	report = []
	for brain in result:
		obj = brain.getObject()
		report.append(str(obj.getId()) +'--'+ str(obj.updateAllRank()))
	return reports
