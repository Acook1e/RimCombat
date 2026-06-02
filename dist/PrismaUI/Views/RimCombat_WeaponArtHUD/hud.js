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

const hudState = {
    mode: 0,
    name: "",
    statusText: "",
};

function resolveStatusText(mode) {
    if (mode === 2) {
        return hudText.enabled;
    }
    if (mode === 1) {
        return hudText.preparing;
    }
    return hudText.disabled;
}

function renderHud() {
    const isDisabled = hudState.mode === 0;
    const isPrepare = hudState.mode === 1;
    const isEnabled = hudState.mode === 2;

    elements.label.textContent = hudText.label;
    elements.name.textContent = hudState.name || hudText.defaultName;
    elements.status.textContent = hudState.statusText || resolveStatusText(hudState.mode);
    elements.shell.classList.toggle("is-enabled", isEnabled);
    elements.shell.classList.toggle("is-prepare", isPrepare);
    elements.shell.classList.toggle("is-disabled", isDisabled);
}

function setHudState(payload) {
    try {
        const data = JSON.parse(payload);
        hudState.mode = Number(data.state ?? 0);
        hudState.statusText = typeof data.text === "string" ? data.text : "";
        renderHud();
    } catch (error) {
        console.error("WeaponArtHUD: failed to parse state", error, payload);
    }
}

function setHudName(payload) {
    try {
        const data = JSON.parse(payload);
        hudState.name = typeof data.name === "string" ? data.name : "";
        renderHud();
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
        if (data.strings && typeof data.strings === "object") {
            Object.assign(hudText, data.strings);
        }
        if (typeof data.label === "string" && data.label) {
            hudText.label = data.label;
        }
        if (typeof data.defaultName === "string" && data.defaultName) {
            hudText.defaultName = data.defaultName;
        }
        renderHud();
    } catch (error) {
        console.error("WeaponArtHUD: failed to parse config", error, payload);
    }
}

window.setHudState = setHudState;
window.setHudName = setHudName;
window.setHudConfig = setHudConfig;

renderHud();
