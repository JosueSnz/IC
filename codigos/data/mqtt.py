import paho.mqtt.client as mqtt
import json
import sqlite3
from datetime import datetime
import time

## broker.hivemq.com

MQTT_BROKER_HOST = "test.mosquitto.org" 
MQTT_PORT = 1883
MQTT_TOPIC = "UNIFEI/EE/LABTEL/DADOS_JSON"
CLIENT_ID = "banco"

DB_FILE = "micro_data.db"

db_conn = None 

def setup_database():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS leituras (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME NOT NULL,
            valor_analogico REAL,
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

        print(f"[{current_time.strftime('%H:%M:%S')}] INSERIDO | V={tensao:.3f}V, I={corrente:.3f}A")

    except Exception as e:
        print(f"Erro ao inserir dados no SQLite ou JSON inválido: {e}")

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Conectado {MQTT_BROKER_HOST}:{MQTT_PORT}.")
        client.subscribe(MQTT_TOPIC)
        print(f"Tópico: {MQTT_TOPIC}")
    else:
        print(f"Falha na conexão. Código de erro: {rc}")

def on_message(client, userdata, msg):
    global db_conn
    payload_str = msg.payload.decode()
    try:
        dados_json = json.loads(payload_str)
        insert_data(db_conn, dados_json)
        
    except json.JSONDecodeError:
        print(f"Erro ao decodificar JSON: {payload_str}")

if __name__ == "__main__":
    print("Iniciando script Assinante MQTT...")
    
    db_conn = setup_database()
    
    mqtt_client = mqtt.Client(client_id=CLIENT_ID, transport="tcp")
    
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    
    try:
        mqtt_client.connect(MQTT_BROKER_HOST, MQTT_PORT, 60)
        mqtt_client.loop_forever() 

    except KeyboardInterrupt:
        print("\nPrograma interrompido.")
    except Exception as e:
        print(f"Erro fatal: {e}")
        
    finally:
        if db_conn:
            db_conn.close()
            print("Conexão com o banco de dados fechada.")