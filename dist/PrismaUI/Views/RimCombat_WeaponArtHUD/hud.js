const elements = {
    anchor: document.getElementById("hud-anchor"),
    shell: document.getElementById("hud-shell"),
    name: document.getElementById("hud-name"),
    label: document.getElementById("hud-label"),
    status: document.getElementById("hud-status"),
};

function setHudState(payload) {
    try {
        const data = JSON.parse(payload);
        const enabled = Boolean(data.enabled);

        elements.status.textContent = enabled ? "Enabled" : "Disabled";
        elements.shell.classList.toggle("is-enabled", enabled);
        elements.shell.classList.toggle("is-disabled", !enabled);
    } catch (error) {
        console.error("WeaponArtHUD: failed to parse state", error, payload);
    }
}

function setHudName(payload) {
    try {
        const data = JSON.parse(payload);
        elements.name.textContent = data.name || "Unknown Weapon Art";
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
    } catch (error) {
        console.error("WeaponArtHUD: failed to parse config", error, payload);
    }
}

window.setHudState = setHudState;
window.setHudName = setHudName;
window.setHudConfig = setHudConfig;
