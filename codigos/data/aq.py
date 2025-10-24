import requests
import sqlite3
import time
from datetime import datetime

# --- Configurações ---
URL = "2adf3f59fd3d4f59a45b94dd1809ffcd.s1.eu.hivemq.cloud:8883"  #colocar o ip certo
DB_FILE = "micro_data.db"         

# --- Funções do Banco de Dados ---

def setup_database():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS leituras (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME NOT NULL,
            valor_analogico INTEGER,
            tensao_medida REAL,      
            corrente_medida REAL
        )
    ''')
    conn.commit()
    return conn

def insert_data(conn, data):
    try:
        cursor = conn.cursor()
        current_time = datetime.now()

        tensao = data['tensao_medida']
        analog_value = data['valor_analogico']
        corrente = data['corrente_medida']

        sql = """
                INSERT INTO leituras 
                (timestamp, valor_analogico, tensao_medida, corrente_medida) 
                VALUES (?, ?, ?, ?)
            """

        cursor.execute(sql, (current_time, analog_value, tensao, corrente))
        conn.commit()

        print(f"[{current_time.strftime('%H:%M:%S')}] Inserido: {analog_value} (V={tensao:.3f}V, I={corrente:.3f}A)")

    except sqlite3.Error as e:
        print(f"Erro ao inserir dados no SQLite: {e}")
    except KeyError as e:
        print(f"Erro: A chave {e} não foi encontrada no JSON recebido.")


def fetch_and_store_data(db_connection):
    try:
        response = requests.get(URL, timeout=5)
        if response.status_code == 200:
            data = response.json()
            insert_data(db_connection, data)
        else:
            print(f"Erro no servidor: Status {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"Erro de conexão: {e}")
    except ValueError:
        print("Erro: Não foi possível decodificar a resposta JSON.")

# --- Loop Principal ---

if __name__ == "__main__":
    print("Iniciando script de coleta de dados...")
    db_conn = setup_database()
    
    try:
        while True:
            fetch_and_store_data(db_conn)
            # Espera 1 segundos antes da próxima coleta
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nPrograma interrompido pelo usuário.")
    finally:
        # Garante que a conexão com o banco de dados seja fechada ao sair
        if db_conn:
            db_conn.close()
            print("Conexão com o banco de dados fechada.")