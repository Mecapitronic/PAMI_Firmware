# PAMI_Firmware
Firmware des Robots PAMI sur ESP32

## Aide des commandes

Envoyer `Help` sur la console pour afficher les commandes enregistrées par les
modules initialisés, ainsi que la légende de leur syntaxe.

```text
Commande:<nom:type@unite>(:nom:type@unite=defaut) - Description
```

- `<...>` : paramètre obligatoire.
- `(...)` : paramètre facultatif, avec son séparateur `:`.
- `type` : `int` (entier signé 32 bits), `string` (texte), `bool` (`0` ou `1`).
  `int|string` accepte l'un ou l'autre. Les flottants ne sont pas analysés nativement.
- `@unite` : unité attendue, par exemple `ms`, `%`, `deg`, `mm` ; omise si sans objet.
- `=defaut` : valeur utilisée lorsque le paramètre est omis, si applicable.

Les annotations ne se saisissent pas. Par exemple :

| Aide | Commande à envoyer |
|---|---|
| `Delay:<duration:int@ms>` | `Delay:1000` |
| `PrefShow:<key:string>:<defaultValue:int|string>` | `PrefShow:Speed:50` |
| `AX12Wait(:id:int)` | `AX12Wait` ou `AX12Wait:1` |
| `AX12Torque:<id:int>:<torque:int@%>` | `AX12Torque:1:30` |

Terminer chaque commande par un retour à la ligne ou `;`. Les types, unités et
contraintes décrivent les paramètres ; ils n'ajoutent pas de validation automatique.
Les chaînes entièrement numériques sont interprétées comme des entiers, et aucun
échappement des séparateurs `:` et `;` n'est disponible.

Voir la [convention complète et les limites du parseur](lib/Esp32_Helper/README.md#aide-et-syntaxe-des-commandes)
et les [commandes matérielles AX12 et Match](lib/Esp32_Hardware/README.md#commandes-console).
