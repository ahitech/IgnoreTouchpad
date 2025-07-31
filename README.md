# Ignore Touchpad

**Ignore Touchpad** is a utility suite for [Haiku OS](http://www.haiku-os.org) that allows users to selectively ignore input from specific pointing devices, such as touchpads or external mice.  

Its main purpose is to prevent accidental cursor movements and clicks while typing, for example when your thumbs touch the laptop's touchpad.

---

## ✨ Features

- Temporarily **ignore input from selected devices** (touchpads or mice).
- Never blocks keyboards or other non-pointing input devices.
  - But this may be changed in the future, especially if users request this functionality.
- System-wide effect, implemented via an `input_addon`. You can read more about input add-ons in the [relevant BeOS Book chapter](https://www.haiku-os.org/legacy-docs/bebook/BInputServerFilter_Overview.html).
- **Safety-first design**:  
  - Settings are stored in a separate configuration file.  
  - Booting into safe mode with "Disable user add-ons" automatically disables the add-on and therefore re-enables all devices.
- Global keyboard shortcut to **unignore all devices** instantly. Assuming keyboard is never affected by this program, a shortcut should be a safe way to revert current status and make ~~Haiku great~~ all devices available again.
- Optional **Deskbar replicant** to show and manage current ignore status.
- CLI and GUI utilities to manage device settings.

---

## 📂 Project Structure

TBD

---

## ⚙️ How It Works

1. The `input_addon` monitors all pointing devices registered in the system.
2. If a device is listed in the **ignore list** (stored in `~/config/settings/IgnoreTouchpad`),  
   all input events from that device are silently discarded.
3. The GUI (`Ignore Touchpad Settings`) allows you to:
   - View all connected pointing devices.
   - Toggle "Ignore input" per device.
   - Restore all devices to the default (unignored) state with one button.
4. The CLI (`ignore_touchpad`) supports scripting and automation:

```bash
ignore_touchpad --list
ignore_touchpad --ignore <device_id>
ignore_touchpad --unignore <device_id>
ignore_touchpad --unignore-all
```

5. An optional **Deskbar replicant** shows the current status and quick actions.

---

## 🔐 Safety Considerations

- The **last active pointing device** cannot be ignored (checkbox is disabled in UI, the CLI command will fail).  
- Booting into Haiku **Safe Mode** with *Disable user add-ons* restores all devices.  
- Settings are per-user, (which is cool as there's only one user in the system), and resettable via the CLI or GUI.

---

## 🛠️ Building

This project uses **autotools**:

```bash
git clone git@github.com:yourusername/ignore_mouse.git
cd ignore_mouse
./configure
make
make install
```

---

## 📄 License

This project is licensed under the MIT License. See License.md.

---

## ✅ Implemented / 🚧 ToDo

🚧 Project structure and initial design
✅ MIT License and README
🚧 `lib/settings` shared library
🚧 CLI utility ignore_touchpad
🚧 input_addon for event filtering
🚧 GUI app for managing device settings and preferences
🚧 Deskbar replicant with status icon
🚧 Translations (CatKeys)
🚧 Install/uninstall scripts
🚧 Scenarios
  - Always disable touchpad when a known pointing device such as external mouse is connected
  - Automatically enable touchpad when the external pointing device is disconnected

Happy typing without accidental touchpad clicks! 
