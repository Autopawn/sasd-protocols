import numpy as np

# Programa del algoritmo de Fast Event Ordering para comprobar si la elección arbitraria del proceso de referencia implica cambios importantes respecto a los retrazos puestos al recibir los paquetes.

# Recibe una tabla con los retrasos entre cada conexión y ejecuta el algoritmo para ver el retraso final que se le pondrá a la recepción de los paquetes.
def tss_table(delay_table):
    n,_ = delay_table.shape
    for pref in range(n-1,-1,-1):
        difs = np.zeros(n)
        for pi in range(n):
            if pi==pref: continue
            max_dif = -1
            for pj in range(n):
                dif_temp = delay_table[pj][pi]-delay_table[pj][pref]
                if dif_temp > max_dif:
                    max_dif = dif_temp
            difs[pi] = max_dif
        if not np.min(difs)<=0: continue
        # Calcular los ajustes a cada nodo:
        adjusts = np.zeros((n,n))
        recv_delay = np.zeros((n,n))
        for myid in range(n):
            for pi in range(n):
                adjusts[myid][pi] = difs[pi]-(delay_table[myid][pi]-delay_table[myid][pref])
                # Calcular el retraso a los paquetes recibidos:
                recv_delay[myid][pi] = delay_table[myid][pi]+adjusts[myid][pi]
        # Imprimir los retrasos:
        # if np.min(recv_delay)<0: continue;
        print("Table for pref = %d"%pref)
        print(recv_delay)

# Create random martrix of delays
n = 4
delay_table = (5*np.random.random((n,n))).astype(int)
delay_table = np.abs(delay_table-delay_table.T)+1

delay_table = np.array([[1,3,4,1],[3,1,2,2],[4,2,1,3],[1,2,3,1]])

print("Matrix of delays:")
print(delay_table)
tss_table(delay_table)
