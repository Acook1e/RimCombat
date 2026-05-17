const elements = {
    anchor: document.getElementById("hud-anchor"),
    shell: document.getElementById("hud-shell"),
    name: document.getElementById("hud-name"),
    label: document.getElementById("hud-label"),
    status: document.getElementById("hud-status"),
};

const hudText = {
    label: "Weapon Art",
    defaultName: "Unassigned",
    disabled: "Disabled",
    preparing: "Preparing",
    enabled: "Enabled",
};

function setHudState(payload) {
    try {
        const data = JSON.parse(payload);
        const state = Number(data.state ?? 0);

        const isDisabled = state === 0;
        const isPrepare = state === 1;
        const isEnabled = state === 2;

        elements.status.textContent =
            data.text || (isEnabled ? hudText.enabled : isPrepare ? hudText.preparing : hudText.disabled);
        elements.shell.classList.toggle("is-enabled", isEnabled);
        elements.shell.classList.toggle("is-prepare", isPrepare);
        elements.shell.classList.toggle("is-disabled", isDisabled);
    } catch (error) {
        console.error("WeaponArtHUD: failed to parse state", error, payload);
    }
}

function setHudName(payload) {
    try {
        const data = JSON.parse(payload);
        elements.name.textContent = data.name || hudText.defaultName;
    } catch (error) {
        console.error("WeaponArtHUD: failed to parse name", error, payload);
    }
}

function setHudConfig(payload) {
    try {
        const data = JSON.parse(payload);

        if (typeof data.x === "number") {
            document.documentElement.style.setProperty("--hud-x", `${data.x}%`);
        }
        if (typeof data.y === "number") {
            document.documentElement.style.setProperty("--hud-y", `${data.y}%`);
        }
        if (typeof data.scale === "number") {
            document.documentElement.style.setProperty("--hud-scale", String(data.scale));
        }
        if (typeof data.label === "string" && data.label) {
            hudText.label = data.label;
            elements.label.textContent = data.label;
        }
        if (typeof data.defaultName === "string" && data.defaultName) {
            hudText.defaultName = data.defaultName;
            if (!elements.name.textContent) {
                elements.name.textContent = data.defaultName;
            }
        }
    } catch (error) {
        console.error("WeaponArtHUD: failed to parse config", error, payload);
    }
}

window.setHudState = setHudState;
window.setHudName = setHudName;
window.setHudConfig = setHudConfig;
