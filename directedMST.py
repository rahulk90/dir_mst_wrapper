import numpy as np
import os
def getDirectedSpanningTree(edgeList,N,fname,resultFile,mstwrapperloc='./'):
	#Append to beginning of file
	f = open(fname,'w')
	f.write(str(N)+"\n")
	np.savetxt(f,edgeList,delimiter=',',fmt=['%d','%d','%d'])
	f.close()	
	os.system(mstwrapperloc+'mstwrapper '+fname+' '+resultFile)
if __name__ == '__main__':
	t = np.zeros((3,3))
	t[0,:]=np.array([1,2,12])
	t[1,:]=np.array([2,3,321])
	t[2,:]=np.array([3,1,5])
	getDirectedSpanningTree(t,3,'smallTest.txt','smallResult.txt')
