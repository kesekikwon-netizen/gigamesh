import sys
import pandas as pd
from numpy.linalg import eig
import numpy as np

def main():
    #first argv is always the program name
    if len(sys.argv) < 2:
        print("ERR1: no path passed as argument ")
        return 1

    verticesCsvPath = sys.argv[1]
    try:
        df = pd.read_csv(verticesCsvPath, header=None)
    except:
        print("ERR2: It's not possible to read the CSV")
        return 2
    #print(df)
    #delete indices 
    positions = df.drop([df.columns[0]], axis=1)
    #calculate PCA 
    # calculate the center of mass of the mesh
    center_of_mass = np.mean(positions, axis=0)

    # calculate the covariance matrix
    cov_matrix = np.cov(positions, rowvar=False)

    # calculate the eigenvalues and eigenvectors
    eigenvalues, eigenvectors = np.linalg.eigh(cov_matrix)

    # sorting eigenvectors based on their eigenvalues
    eig_values_index = np.argsort(eigenvalues)[::-1]
    eig_vectors = eigenvectors[:, eig_values_index]

    # save center of mass as np array
    center_of_mass = np.array(center_of_mass)

    # create the PCA transformation matrix
    trafomat = np.vstack((np.vstack((eig_vectors,center_of_mass)).T, np.array([0,0,0,1])))


    #overwrite the input csv with the principal components
    resultDf = pd.DataFrame(trafomat)
    resultDf.to_csv(verticesCsvPath)

if __name__=="__main__":
    main()
