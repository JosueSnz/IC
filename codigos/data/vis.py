import sqlite3
import time
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.dates import DateFormatter, MinuteLocator

DB_FILE = "micro_data.db"
INTERVALO_MS = 1000
MAX_PONTOS = 60 

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8)) 

def animate(i):
    conn = None
    try:
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        
        cursor.execute(f'''
            SELECT timestamp, tensao_medida, corrente_medida
            FROM leituras
            ORDER BY id DESC
            LIMIT {MAX_PONTOS}
        ''')
        
        rows = cursor.fetchall()[::-1] 
        
        times = []
        tensoes = []
        correntes = []

        for row in rows:
            times.append(datetime.strptime(row[0], '%Y-%m-%d %H:%M:%S.%f'))
            tensoes.append(row[1])
            correntes.append(row[2])
        
        ax1.clear()
        ax1.plot(times, tensoes, color='blue', label='Tensão Medida [V]')
        ax1.set_title('Monitoramento em Tempo Real')
        ax1.set_ylabel('Tensão [V]')
        ax1.grid(True, linestyle='--', alpha=0.6)
        
        ax2.clear()
        ax2.plot(times, correntes, color='red', label='Corrente Calculada [A]')
        ax2.set_ylabel('Corrente [A]')
        ax2.set_xlabel('Tempo')
        ax2.grid(True, linestyle='--', alpha=0.6)
        
        date_form = DateFormatter('%H:%M:%S')
        ax1.xaxis.set_major_formatter(date_form)
        ax2.xaxis.set_major_formatter(date_form)
        
        fig.autofmt_xdate() 
        
    except sqlite3.Error as e:
        print(f"Erro ao acessar o banco de dados: {e}")
    finally:
        if conn:
            conn.close()

if __name__ == "__main__":
    print(f"Iniciando Visualizador Supervisório. Atualização a cada {INTERVALO_MS}ms.")
    
    ani = animation.FuncAnimation(fig, animate, interval=INTERVALO_MS)
    plt.show()