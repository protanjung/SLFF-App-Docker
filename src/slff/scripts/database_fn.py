#!/usr/bin/python3

import datetime

import database_sql as db_sql


def replace_tbl_rfid_tag(db, dbCursor, msg):
    sql = db_sql.sql_replace_tbl_rfid_tag
    val = (
        str(msg.epc.hex().upper()),
        str(msg.tid.hex().upper()),
        str(msg.userdata.hex().upper())
    )
    dbCursor.execute(sql, val)
    db.commit()
    return


def replace_tbl_gto_present(db, dbCursor, msg):
    sql = db_sql.sql_replace_tbl_gto_present
    val = (
        msg.no_seri_control_unit,
        msg.rfid_tid,
        msg.golongan_kendaraan,
        msg.jenis_kendaraan,
        msg.plat_no_rss,
        msg.plat_no_anpr,
        msg.saldo,
        msg.tarif,
        msg.no_gardu_entrance,
        msg.no_gerbang_entrance,
        msg.no_gardu_exit,
        msg.no_gerbang_exit,
        None if msg.entrance_year == 0 else '{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}'.format(
            msg.entrance_year + 2000,
            msg.entrance_month,
            msg.entrance_day,
            msg.entrance_hour,
            msg.entrance_minute,
            msg.entrance_second
        ),
        msg.hash
    )
    dbCursor.execute(sql, val)
    db.commit()


def replace_tbl_gto_notification(db, dbCursor, msg):
    sql = db_sql.sql_replace_tbl_gto_notification
    val = (
        msg.no_seri_control_unit,
        msg.rfid_tid,
        msg.golongan_kendaraan,
        msg.message.replace('\n', ' ')
    )
    dbCursor.execute(sql, val)
    db.commit()


def replace_tbl_gto_store(db, dbCursor, msg):
    sql = db_sql.sql_replace_tbl_gto_store
    val = (
        msg.no_seri_control_unit if msg.metode_pembayaran == 0 else msg.no_resi * -1,
        msg.metode_pembayaran,
        msg.rfid_tid,
        None if msg.entrance_year == 0 else '{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}'.format(
            msg.entrance_year + 2000,
            msg.entrance_month,
            msg.entrance_day,
            msg.entrance_hour,
            msg.entrance_minute,
            msg.entrance_second
        ),
        None if msg.exit_year == 0 else '{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}'.format(
            msg.exit_year + 2000,
            msg.exit_month,
            msg.exit_day,
            msg.exit_hour,
            msg.exit_minute,
            msg.exit_second
        ),
        None if msg.report_year == 0 else '{:04d}-{:02d}-{:02d}'.format(
            msg.report_year + 2000,
            msg.report_month,
            msg.report_day
        ),
        msg.kode_ruas,
        msg.no_shift,
        msg.no_perioda,
        msg.no_resi,
        msg.no_kspt,
        msg.no_plt,
        msg.hash
    )
    dbCursor.execute(sql, val)
    db.commit()
