import sys
import os
import json
from somfy_protect_api.api.somfy_protect_api import SomfyProtectApi
from oauthlib.oauth2.rfc6749.errors import InvalidGrantError, TokenExpiredError

# Chemin vers ~/.SProtect et le cache des tokens au même endroit
CONFIG_DIR = os.path.expanduser("~")
CONFIG_PATH = os.path.join(CONFIG_DIR, ".SProtect")
CACHE_PATH = os.path.join(CONFIG_DIR, ".SProtect_token.json")

def load_config():
    """Charge les identifiants depuis le fichier ~/.SProtect."""
    if not os.path.exists(CONFIG_PATH):
        print(f"❌ Erreur : Le fichier de configuration '{CONFIG_PATH}' est introuvable.")
        print("Veuillez créer ce fichier au format JSON avec vos identifiants Somfy.")
        sys.exit(1)
    
    try:
        with open(CONFIG_PATH, "r") as f:
            config = json.load(f)
            return config.get("username"), config.get("password")
    except Exception as e:
        print(f"❌ Erreur lors de la lecture de '{CONFIG_PATH}' : {e}")
        sys.exit(1)

USERNAME, PASSWORD = load_config()

def print_help():
    """Affiche l'aide de l'utilitaire."""
    print("==================================================")
    print("  Gestion Somfy Protect - Domestik Micro-Bridge")
    print("==================================================")
    print("Utilisation : python3 SProtect.py [COMMANDE] [OPTIONS]")
    print("\nCommandes disponibles :")
    print("  status                  Affiche le mode actuel et l'état de l'alarme")
    print("  disarmed                Désactive l'alarme")
    print("  partial                 Active le mode Nuit / Partiel")
    print("  armed                   Active le mode Total (Full)")
    print("  trigger [mode]          Déclenche l'alarme (modes: alarm, silent, light)")
    print("                          [défaut du mode si omis : 'alarm']")
    print("  stop (ou silence)       Arrête la sirène en cours")
    print("  help                    Affiche cette aide")
    print("==================================================")

def get_token():
    try:
        if os.path.exists(CACHE_PATH):
            with open(CACHE_PATH, "r") as f:
                return json.loads(f.read())
    except Exception:
        pass
    return None

def set_token(token):
    try:
        with open(CACHE_PATH, "w") as f:
            f.write(json.dumps(token))
    except Exception as e:
        print(f"Avertissement : impossible d'écrire le token ({e})")

def clear_token():
    if os.path.exists(CACHE_PATH):
        try:
            os.remove(CACHE_PATH)
            print("ℹ️ Token expiré ou invalide supprimé du cache.")
        except OSError:
            pass

def init_api():
    token = get_token()
    api = SomfyProtectApi(
        username=USERNAME,
        password=PASSWORD,
        token=token,
        token_updater=set_token
    )

    if token is None:
        print("Authentification auprès de Somfy...")
        new_token = api.request_token()
        set_token(new_token)

    return api

def execute_with_retry(api_func):
    """Exécute une commande. Si le token OAuth est expiré, re-tente une seule fois après purge."""
    try:
        api = init_api()
        return api_func(api)
    except (InvalidGrantError, TokenExpiredError) as e:
        print(f"⚠️ Session OAuth expirée ({type(e).__name__}). Réinitialisation...")
        clear_token()
        time.sleep(1) # Courte pause pour éviter le spam
        api = init_api()
        return api_func(api)
    except Exception as e:
        print(f"❌ Erreur lors de l'exécution : {e}")

def manage_alarm(args):
    action = args[0] if len(args) > 0 else None

    if action is None or action == "help":
        print_help()
        return

    def _action(api):
        sites = api.get_sites()
        if not sites:
            print("Erreur: Aucun site trouvé sur ce compte.")
            return

        site_id = sites[0].id

        # 1. Action : STATUS
        if action == "status":
            site_details = api.get_site(site_id)
            current_status = site_details.security_level
            is_alarm_sounding = getattr(site_details, "in_alarm", False) or getattr(site_details, "status", "ok") != "ok"
            
            print(f"Statut armement  : {current_status}")
            print(f"Alarme en cours  : {'🚨 OUI' if is_alarm_sounding else 'NON'}")
            return

        # 2. Action : TRIGGER (mode: "alarm", "silent", "light")
        if action == "trigger":
            trigger_mode = args[1] if len(args) > 1 else "alarm"
            valid_triggers = ["alarm", "silent", "light"]
            
            if trigger_mode not in valid_triggers:
                print(f"Mode de déclenchement invalide : '{trigger_mode}'. Modes valides : {valid_triggers}")
                return

            print(f"🚨 Déclenchement de l'alarme (mode: {trigger_mode})...")
            api.trigger_alarm(site_id=site_id, mode=trigger_mode)
            print("✅ Ordre de déclenchement envoyé !")
            return

        # 3. Action : STOP
        if action in ["stop", "silence"]:
            print("🔕 Arrêt de la sirène...")
            api.stop_alarm(site_id=site_id)
            print("✅ Sirène arrêtée.")
            return

        # 4. Action : DISARMED / PARTIAL / ARMED
        valid_modes = ["disarmed", "partial", "armed"]
        if action in valid_modes:
            current_status = sites[0].security_level
            if current_status == action:
                print(f"L'alarme est déjà en mode : {action}")
            else:
                api.update_security_level(site_id=site_id, security_level=action)
                print(f"✅ Alarme passée en mode : {action}")
            return

        print(f"Commande inconnue: '{action}'")
        print_help()

    execute_with_retry(_action)

if __name__ == "__main__":
    manage_alarm(sys.argv[1:])
