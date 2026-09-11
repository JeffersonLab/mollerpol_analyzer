import mysql.connector
import epics  # Or pyepics / caget module depending on your setup

# 1. Database connection settings
db_config = {
    'host': 'localhost',
    'user': 'your_username',
    'password': 'your_password',
    'database': 'your_database'
}

def process_epics_run(pv_file_path, run_number):
    pv_map = {}
    
    # 2. Read the input file and map PV names to database column names
    with open(pv_file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split()
            if len(parts) >= 2:
                pv_name = parts[0]
                db_col = parts[1]
                
                # Skip entries not mapped in the database schema
                if db_col.upper() != 'N/A':
                    pv_map[db_col] = pv_name

    # 3. Query EPICS for each PV
    pv_values = {}
    for db_col, pv_name in pv_map.items():
        try:
            val = epics.caget(pv_name)
            pv_values[db_col] = val
        except Exception as e:
            print(f"Error fetching {pv_name}: {e}")
            pv_values[db_col] = None

    # 4. Construct dynamic SQL INSERT query with ON DUPLICATE KEY UPDATE
    columns = ['run_number'] + list(pv_values.keys())
    placeholders = ['%s'] * len(columns)
    
    update_assignments = [f"{col} = VALUES({col})" for col in pv_values.keys()]
    
    sql = f"""
        INSERT INTO EPICS_data ({', '.join(columns)})
        VALUES ({', '.join(placeholders)})
        ON DUPLICATE KEY UPDATE {', '.join(update_assignments)};
    """
    
    values = [run_number] + [pv_values[col] for col in pv_values.keys()]

    # 5. Execute DB transaction
    conn = mysql.connector.connect(**db_config)
    cursor = conn.cursor()
    try:
        cursor.execute(sql, values)
        conn.commit()
        print(f"Successfully recorded EPICS data for run {run_number}.")
    except mysql.connector.Error as err:
        print(f"Database error: {err}")
        conn.rollback()
    finally:
        cursor.close()
        conn.close()

# Example usage:
# process_epics_run('epics_pv_list.txt', run_number=12345)
