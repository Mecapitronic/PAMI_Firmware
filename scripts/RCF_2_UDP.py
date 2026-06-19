#!/usr/bin/env python3

import argparse
import time
from pathlib import Path
from sys import platform
from typing import List

from serial import Serial
import serial
import socket

teleplotLocal = ("127.0.0.1", 47269)
teleplotRemote = ("teleplot.fr", 60964)
teleplotSendTo = teleplotLocal
sockUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)
sockTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
serialPort = None

DEBUG = True

BAUDRATE = 921600
SERIAL_LNX = "/dev/ttyUSB"
SERIAL_WIN = "COM"
OPEN_RETRY_DELAY_S = 3


def log(level: str, message: str) -> None:
    if not DEBUG and level == "DEBUG":
        return
    now = time.strftime("%H:%M:%S")
    print(f"[{now}] [{level}] {message}")


def init_serial(serial_if: str) -> None:
    global serialPort
    attempts = 0
    while True:
        attempts += 1
        try:
            log(
                "INFO",
                f"Ouverture interface serie (tentative {attempts}): {serial_if} @ {BAUDRATE} bauds",
            )
            if serial_if.startswith("rfc2217://"):
                serialPort = serial.serial_for_url(serial_if, BAUDRATE)
            else:
                serialPort = serial.Serial(serial_if, BAUDRATE)
            serialPort.reset_input_buffer()
            serialPort.reset_output_buffer()
            log(
                "INFO",
                f"Connexion serie etablie (port={serialPort.port}, baudrate={serialPort.baudrate})",
            )
            return
        except Exception as exc:
            log("ERROR", f"Erreur ouverture serie: {exc}")
            log(
                "INFO",
                f"Nouvelle tentative dans {OPEN_RETRY_DELAY_S}s...",
            )
            time.sleep(OPEN_RETRY_DELAY_S)


def send_packet(packet: bytes) -> None:
    global serialPort
    if serialPort is not None:
        serialPort.write(packet)

def packet_available() -> bool:
    global serialPort
    # Laisse remonter SerialException/OSError pour que main() gere la reconnexion
    if serialPort is not None and serialPort.in_waiting > 0:
        return True
    return False

def get_packet() -> bytes | None:
    global serialPort
    # Laisse remonter SerialException pour que main() gere la reconnexion
    if serialPort is not None and serialPort.in_waiting > 0:
        packet = serialPort.readline()
        return packet.replace(b'\r\n',b'\n')
    return None


def close_connections() -> None:
    global serialPort
    try:
        if serialPort is not None and serialPort.is_open:
            serialPort.close()
            log("INFO", "Connexion serie fermee")
    except Exception as exc:
        log("ERROR", f"Erreur fermeture serie: {exc}")

    try:
        sockUDP.close()
        log("INFO", "Socket UDP fermee")
    except Exception as exc:
        log("ERROR", f"Erreur fermeture socket UDP: {exc}")

    try:
        sockTCP.close()
        log("INFO", "Socket TCP fermee")
    except Exception as exc:
        log("ERROR", f"Erreur fermeture socket TCP: {exc}")


def which_serial(serial_if: str | None = None, serial_num: int = 0) -> str:
    if serial_if:
        return serial_if

    if os_is_unix():
        return SERIAL_LNX + str(serial_num)
    else:
        return SERIAL_WIN + str(serial_num)

def os_is_unix() -> bool:
    if platform == "linux" or platform == "linux2" or platform == "darwin":
        return True
    else:
        return False


def packets_from_file(data_file: Path) -> List:
    with open(data_file, "r") as f:
        data = f.readlines()
    return data


def main(serial_if: str):
    log("INFO", f"Demarrage ecoute -> {serial_if}")
    log("INFO", f"Destination UDP Teleplot -> {teleplotSendTo[0]}:{teleplotSendTo[1]}")
    init_serial(serial_if)
    while True:
        try:
            if packet_available():
                packet = get_packet()
                if packet is None:
                    continue
                decoded_string = packet.decode("utf-8")
                decoded_string = decoded_string.replace("\n","")
                decoded_string = decoded_string.replace("\r","")
                if decoded_string.startswith(">"):
                    decoded_string = decoded_string.replace(">","",1)
                else:
                    decoded_string += "|np"
                sockUDP.sendto(decoded_string.encode(), teleplotSendTo)
        except KeyboardInterrupt:
            log("INFO", "Interruption utilisateur, fermeture en cours")
            close_connections()
            break
        except (serial.SerialException, OSError) as exc:
            log("ERROR", f"Connexion serie perdue: {exc}")
            try:
                if serialPort is not None:
                    serialPort.close()
            except Exception:
                pass
            log("INFO", f"Reconnexion dans {OPEN_RETRY_DELAY_S}s...")
            time.sleep(OPEN_RETRY_DELAY_S)
            init_serial(serial_if)
        except UnicodeDecodeError as exc:
            log("ERROR", f"Erreur decoding UTF-8: {exc}")
        except Exception as exc:
            log("ERROR", f"Erreur inattendue dans la boucle principale: {exc}")

if __name__ == "__main__":
    argParser = argparse.ArgumentParser()
    argParser.add_argument(
        "interface_type",
        type=str,
        default='socket',
        nargs="?",
        help="type of interface, serial or socket",
    )
    argParser.add_argument(
        "interface",
        type=str,
        default='rfc2217://localhost:4000',
        nargs="?",
        help="interface where to send packets",
    )
    args = argParser.parse_args()

    serial_if = which_serial(serial_if=args.interface)
    main(serial_if)
