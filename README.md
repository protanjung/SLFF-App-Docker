<p align="center">
    <img src="https://i.imgur.com/S8pm7jM.png" width="200" height="200">
</p>

**GitHub Stat(s):**  
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/protanjung/slff-app-docker?logo=github) ![GitHub all releases](https://img.shields.io/github/downloads/protanjung/slff-app-docker/total?logo=github) [![Build Status](https://img.shields.io/endpoint.svg?url=https%3A%2F%2Factions-badge.atrox.dev%2Fprotanjung%2Fslff-app-docker%2Fbadge%3Fref%3Dmain&style=flat)](https://actions-badge.atrox.dev/protanjung/slff-app-docker/goto?ref=main)

**DockerHub Stat(s):**  
![Docker Pulls](https://img.shields.io/docker/pulls/protanjung/slff-app-docker?logo=docker) ![Docker Image Size (tag)](https://img.shields.io/docker/image-size/protanjung/slff-app-docker/latest?logo=docker) ![Docker Image Version (latest by date)](https://img.shields.io/docker/v/protanjung/slff-app-docker?arch=amd64&logo=docker&sort=date)

Dengan Rahmat Tuhan Yang Maha Kuasa. Dengan ini kami membuat aplikasi Control Unit untuk transaksi jalan tol tanpa henti menggunakan teknologi RFID. Demikian agar dapat bermanfaat untuk seluruh rakyat Indonesia.

## Preparation
- Membuat folder **~/slff-data**
    ```bash
    mkdir -p ~/slff-data
    ```
- Membuat file **slff.yaml** di folder **~/slff-data**
    ```bash
    touch ~/slff-data/slff.yaml
    ```
- Menulis parameter di file **slff.yaml**
    ```yaml
    # --------------------------------------
    # Semanggi 1 Gardu 2
    # --------------------------------------

    # Tipe Control Unit
    # ==================
    # tipe_control_unit: 1 -> Open
    # tipe_control_unit: 2 -> Entrance
    # tipe_control_unit: 3 -> Exit
    # ==================
    tipe_control_unit: 1

    # Autentikasi
    # ===========
    auth_rfid: true

    # Nomor Gerbang dan Nomor Gardu
    # =============================
    no_gerbang: 10
    no_gardu: 2

    # MID dan TID
    # ===========
    mid: "00000000"
    tid: "00000000"

    # Periph Status Interval
    # ======================
    gto_status_interval: 60
    rfid_status_interval: 60

    # RSS
    # ===
    rss:
        check_url: "0.0.0.0:6000/api/v1/rss/rfid_check"
        store_url: "0.0.0.0:6000/api/v1/rss/store_data"
        jm_code: "00000000000000000000000000000000"

    # Database
    # ========
    # database/host: 'msql'    -> Nama container jika menggunakan docker
    # database/host: '0.0.0.0' -> IP Address jika tidak menggunakan docker
    # ========
    database:
        host: "mysql"
        name: "slff"
        user: "slff"
        password: "slff"

    # Expansion Board
    # ===============
    expansion:
        active: false
        port: "/dev/ttyACM0"
        baud: 1000000

    # RFID
    # ====
    # rfid/type: 0 -> Invengo
    # rfid/type: 1 -> CU1
    # ====
    rfid:
        use_native: true
        port_native: "/dev/ttyS0"
        port: 1
        baud: 115200
        type: 0
        power: 20

    # GTO
    # ===
    gto:
        use_native: true
        port_native: "/dev/ttyS1"
        port: 2
        baud: 19200
    ```
    
        