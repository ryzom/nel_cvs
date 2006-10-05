# -*- coding: utf-8 -*-

def fusion(t1,t2,limit):
	""" fusionne 2 liste selon 'score' et 'rank' """
	i = j = 0
	rk = 1
	result = []
	while len(result) < limit:
		try:
			rank1 = t1[i]
		except IndexError:
			while (len(result) < limit and j < len(t2)):
				rank2 = t2[j]
				rank2['rank'] = rk
				result.append(rank2)
				j+=1
				rk+=1
			return result
		try:
			rank2 = t2[j]
		except IndexError:
			while (len(result) < limit and i < len(t1)):
				rank1 = t1[i]
				rank1['rank'] = rk
				result.append(rank1)
				i+=1
				rk+=1
			return result

		if rank1['score']>=rank2['score']:
			rank1['rank'] = rk
			result.append(rank1)
			i+=1
			rk+=1
		else:
			rank2['rank'] = rk
			result.append(rank2)
			j+=1
			rk+=1
	return result
