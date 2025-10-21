import http.server
import socketserver
import json
import sqlite3
import os
from datetime import datetime
import sys
import threading 
import time   

HOST = "0.0.0.0" 
PORT = 8000 
DB_FILE = "micro_data.db"

MIN_INTERVAL_SECONDS = 0.5
LAST_INSERT_TIME = time.time()
TIME_LOCK = threading.Lock()

def setup_database(db_path):
    try:
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
        conn.close()
        print(f"Banco de dados '{db_path}' e esquema verificados com sucesso.")
    except sqlite3.Error as e:
        print(f"ERRO FATAL: Falha ao configurar o esquema do SQLite: {e}", file=sys.stderr)
        sys.exit(1)

# DB_CONN = setup_database()

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

        print(f"[{current_time.strftime('%H:%M:%S')}] INSERIDO: {analog_value} (V={tensao:.3f}V, I={corrente:.3f}A)")
        return True

    except sqlite3.Error as e:
        print(f"ERRO: Falha ao inserir dados no SQLite: {e}", file=sys.stderr)
        return False
    except KeyError as e:
        print(f"ERRO: Chave '{e}' não encontrada no JSON. JSON Requerido: {{'tensao_medida', 'valor_analogico', 'corrente_medida'}}.", file=sys.stderr)
        return False

class MyRequestHandler(http.server.BaseHTTPRequestHandler):
    
    def do_POST(self):
        global LAST_INSERT_TIME 
        
        current_request_time = time.time()
        should_insert = False
        
        with TIME_LOCK:
            if current_request_time - LAST_INSERT_TIME >= MIN_INTERVAL_SECONDS:
                should_insert = True
                LAST_INSERT_TIME = current_request_time
        
        if not should_insert:
            self.send_response(202) 
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            
            time_diff = current_request_time - LAST_INSERT_TIME
            response = {"status": "ignored", "message": f"Dado descartado. Intervalo de {MIN_INTERVAL_SECONDS:.1f}s não atingido. Última inserção há {time_diff:.3f}s."}
            self.wfile.write(json.dumps(response).encode('utf-8'))
            
            print(f"[{datetime.now().strftime('%H:%M:%S')}] DESCARTADO: Intervalo ({time_diff:.3f}s) < {MIN_INTERVAL_SECONDS:.1f}s.")
            return 

        conn = None 
        try:
            conn = sqlite3.connect(DB_FILE) 
            
            content_length = int(self.headers.get('Content-Length', 0))
            if content_length == 0:
                 raise ValueError("Nenhum dado recebido (Content-Length é zero).")

            post_data = self.rfile.read(content_length)
            data_dict = json.loads(post_data.decode('utf-8'))
            
            if insert_data(conn, data_dict):
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                response = {"status": "success", "message": "Dados recebidos e salvos com sucesso"}
            else:
                self.send_response(500)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                response = {"status": "error", "message": "Falha ao salvar dados no DB."}

            self.wfile.write(json.dumps(response).encode('utf-8'))
            
        except json.JSONDecodeError:
            print("ERRO: O corpo da requisição não é um JSON válido.", file=sys.stderr)
            self._send_error_response(400, "Corpo da requisição não é JSON válido.")
        except Exception as e:
            print(f"ERRO ao processar a requisição POST: {e}", file=sys.stderr)
            self._send_error_response(400, f"Bad Request: {str(e)}")
        finally:
            if conn:
                conn.close() 

    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write("Servidor ok.".encode('utf-8'))

    def _send_error_response(self, code, message):
        self.send_response(code) 
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        response = {"status": "error", "message": message}
        self.wfile.write(json.dumps(response).encode('utf-8'))

if __name__ == "__main__":
    setup_database(DB_FILE)
    
    try:
        s = socketserver.socket(socketserver.AF_INET, socketserver.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
    except:
        local_ip = os.popen('ipconfig | findstr /R "IPv4"').read().splitlines()[0].split(':')[-1].strip()


    print("--- Servidor de Aquisição de Dados (HTTP Padrão) ---")
    print(f"Banco de dados '{DB_FILE}' configurado.")
    print(f"Servidor ouvindo em: http://{local_ip}:{PORT}")
    print("Pressione Ctrl+C para parar o servidor.")

    try:
        # Usa 'http.server.ThreadingHTTPServer' para lidar com múltiplas requisições mais facilmente
        with socketserver.ThreadingTCPServer((HOST, PORT), MyRequestHandler) as httpd:
            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nPrograma interrompido pelo usuário.")
    finally:
        if DB_CONN:
            DB_CONN.close()
            print("Conexão com o banco de dados fechada.")


