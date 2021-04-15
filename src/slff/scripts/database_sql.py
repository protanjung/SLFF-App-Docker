#!/usr/bin/python3

sql_create_tbl_rfid_tag = \
    'CREATE TABLE IF NOT EXISTS `tbl_rfid_tag` (\
        `id` INT NOT NULL AUTO_INCREMENT,\
        `epc` TEXT,\
        `tid` TEXT,\
        `userdata` TEXT,\
        `db_timestamp` TIMESTAMP,\
        `db_flag` BOOLEAN,\
        PRIMARY KEY (`id`)\
    );'

sql_create_tbl_gto_present = \
    'CREATE TABLE IF NOT EXISTS `tbl_gto_present` (\
        `no_seri_control_unit` INT NOT NULL,\
        `rfid_tid` TEXT,\
        `golongan_kendaraan` INT,\
        `jenis_kendaraan` INT,\
        `plat_no_rss` TEXT,\
        `plat_no_anpr` TEXT,\
        `saldo` INT ,\
        `tarif` INT,\
        `no_gardu_entrance` INT,\
        `no_gerbang_entrance` INT,\
        `no_gardu_exit` INT,\
        `no_gerbang_exit` INT,\
        `entrance_datetime` DATETIME,\
        `hash` TEXT,\
        `db_timestamp` TIMESTAMP,\
        `db_flag` BOOLEAN,\
	    PRIMARY KEY (`no_seri_control_unit`)\
    );'

sql_create_tbl_gto_notification = \
    'CREATE TABLE IF NOT EXISTS `tbl_gto_notification` (\
        `no_seri_control_unit` INT NOT NULL,\
        `rfid_tid` TEXT,\
        `golongan_kendaraan` INT,\
        `message` TEXT,\
        `db_timestamp` TIMESTAMP,\
        `db_flag` BOOLEAN,\
        PRIMARY KEY (`no_seri_control_unit`)\
    );'

sql_create_tbl_gto_store = \
    'CREATE TABLE IF NOT EXISTS `tbl_gto_store` (\
        `no_seri_control_unit` INT NOT NULL,\
        `metode_pembayaran` INT,\
        `rfid_tid` TEXT,\
        `entrance_datetime` DATETIME,\
        `exit_datetime` DATETIME,\
        `report_date` DATE,\
        `kode_ruas` INT,\
        `no_shift` INT,\
        `no_perioda` INT,\
        `no_resi` INT,\
        `no_kspt` INT,\
        `no_plt` INT,\
        `hash` TEXT,\
        `db_timestamp` TIMESTAMP,\
        `db_flag` BOOLEAN,\
        PRIMARY KEY (`no_seri_control_unit`)\
    );'

# =============================================================================
# -----------------------------------------------------------------------------
# =============================================================================

sql_create_view_transaction = \
    'CREATE \
    OR REPLACE VIEW view_transaction AS \
    SELECT\
        tgs.rfid_tid,\
        tgp.tarif,\
        tgp.saldo,\
        tgs.db_timestamp,\
        tgs.db_flag \
    FROM\
        tbl_gto_present tgp \
        INNER JOIN\
            tbl_gto_store tgs \
            ON tgp.no_seri_control_unit = tgs.no_seri_control_unit \
    WHERE\
        tgs.metode_pembayaran = 0'\

# =============================================================================
# -----------------------------------------------------------------------------
# =============================================================================

sql_replace_tbl_rfid_tag = \
    'REPLACE INTO `tbl_rfid_tag` (\
        `epc`,\
        `tid`,\
        `userdata`,\
        `db_timestamp`,\
        `db_flag`\
    ) VALUES (%s, %s, %s, NOW(), 0);'

sql_replace_tbl_gto_present = \
    'REPLACE INTO `tbl_gto_present` (\
        `no_seri_control_unit`,\
        `rfid_tid`,\
        `golongan_kendaraan`,\
        `jenis_kendaraan`,\
        `plat_no_rss`,\
        `plat_no_anpr`,\
        `saldo`,\
        `tarif`,\
        `no_gardu_entrance`,\
        `no_gerbang_entrance`,\
        `no_gardu_exit`,\
        `no_gerbang_exit`,\
        `entrance_datetime`,\
        `hash`,\
        `db_timestamp`,\
        `db_flag`\
    ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, NOW(), 0);'

sql_replace_tbl_gto_notification = \
    'REPLACE INTO `tbl_gto_notification` (\
        `no_seri_control_unit`,\
        `rfid_tid`,\
        `golongan_kendaraan`,\
        `message`,\
        `db_timestamp`,\
        `db_flag`\
    ) VALUES (%s, %s, %s, %s, NOW(), 0);'

sql_replace_tbl_gto_store = \
    'REPLACE INTO `tbl_gto_store` (\
        `no_seri_control_unit`,\
        `metode_pembayaran`,\
        `rfid_tid`,\
        `entrance_datetime`,\
        `exit_datetime`,\
        `report_date`,\
        `kode_ruas`,\
        `no_shift`,\
        `no_perioda`,\
        `no_resi`,\
        `no_kspt`,\
        `no_plt`,\
        `hash`,\
        `db_timestamp`,\
        `db_flag`\
    ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, NOW(), 0);'
