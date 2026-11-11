# Somfy Protect Micro-Bridge (SProtect)

A lightweight Python script to control the **Somfy Protect** ecosystem (Alarms, IntelliTAGs, Cameras).

---

## Prerequisites

* **Python 3.8+**
* An active Somfy Protect account.
* The `somfy-protect-api` Python library and its OAuth dependencies.

---

## 1. Installation

1. Clone or place the `SProtect.py` script into your project directory.
2. Create and activate a virtual environment (recommended):
   ```bash
   python3 -m venv venv
   source venv/bin/activate
   ```
3. Install the required dependencies:
   ```bash
   pip install somfy-protect-api oauthlib requests
   ```

---

## 2. Configuration

The script reads your Somfy credentials from a hidden configuration file located in your home directory.

1. Create the `~/.SProtect.json` file:
   ```bash
   vi ~/.SProtect
   ```
2. Add your credentials in JSON format:
   ```json
   {
     "username": "your_somfy_email@example.com",
     "password": "your_password"
   }
   ```
3. *(Optional)* Secure the file permissions:
   ```bash
   chmod 600 ~/.SProtect
   ```

---

## 3. Usage

Run the script passing the desired command:

```bash
python3 SProtect.py [COMMAND] [OPTIONS]
```

### Available Commands:

| Command | Description |
| :--- | :--- |
| `python3 SProtect.py` (or `help`) | Displays CLI help and usage information. |
| `python3 SProtect.py status` | Displays current security level and active alarm state. |
| `python3 SProtect.py disarmed` | Disarms the alarm system. |
| `python3 SProtect.py partial` | Arms the system in Night / Partial mode. |
| `python3 SProtect.py armed` | Arms the system in Full mode. |
| `python3 SProtect.py trigger [mode]` | Manually triggers the alarm (modes: `alarm`, `silent`, `light`). *Default: `alarm`*. |
| `python3 SProtect.py stop` (or `silence`) | Silences an active siren without altering the current security mode. |

---

## 4. Token & Session Management (OAuth2)

OAuth2 access tokens are cached locally at `~/.SProtect_token.json` to minimize requests and prevent hitting Somfy's rate limits.
In the event of token expiration or invalid grants, `SProtect.py` automatically purges the cache and re-authenticates silently.
