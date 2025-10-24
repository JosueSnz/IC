import sqlite3
import csv
from datetime import datetime

DB_FILE = "micro_data.db"
CSV_FILE = "leituras.csv"

def exportar_para_csv():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute("SELECT timestamp, valor_analogico, tensao_medida, corrente_medida FROM leituras")
    rows = cursor.fetchall()

    with open(CSV_FILE, "w", newline="", encoding="utf-8") as csvfile:
        writer = csv.writer(csvfile, delimiter=';')

        writer.writerow(["data", "hora", "valor_analogico", "tensao_medida[V]", "corrente_medida[A]"])

        for row in rows:
            ts = datetime.fromisoformat(row[0])
            data = ts.strftime("%Y-%m-%d")
            hora = ts.strftime("%H:%M:%S")

            writer.writerow([data, hora, row[1], f"{row[2]:.3f}", f"{row[3]:.3f}"])

    conn.close()
    print(f"Exportação concluída: {CSV_FILE}")

if __name__ == "__main__":
    exportar_para_csv()
