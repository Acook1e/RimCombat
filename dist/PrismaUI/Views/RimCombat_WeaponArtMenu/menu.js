const state = {
    playerLevel: 1,
    playerPoint: 0,
    selectedWeapon: null,
    arts: [],
    filterEnabled: false,
};

const elements = {
    artList: document.getElementById("art-list"),
    artCount: document.getElementById("art-count"),
    playerLevel: document.getElementById("player-level"),
    playerPoints: document.getElementById("player-points"),
    weaponPanel: document.getElementById("weapon-panel"),
    detailPanel: document.getElementById("detail-panel"),
    closeButton: document.getElementById("close-button"),
};

let selectedArtId = null;

function callPrismaListener(name, payload = "") {
    const listener = window[name];
    if (typeof listener === "function") {
        listener(payload);
    } else {
        console.info(`[WeaponArtMenu] Listener unavailable: ${name}`, payload);
    }
}

function formatId(id) {
    return typeof id === "number" ? id.toString() : "0";
}

function badge(label, tone = "warn") {
    return `<span class="badge ${tone}">${label}</span>`;
}

function metricCard(label, value) {
    return `<article class="metric-card"><span>${label}</span><strong>${value}</strong></article>`;
}

function normalizeState(nextState) {
    return {
        playerLevel: Number(nextState.playerLevel ?? 1),
        playerPoint: Number(nextState.playerPoint ?? 0),
        selectedWeapon: nextState.selectedWeapon ?? null,
        filterEnabled: Boolean(nextState.filterEnabled),
        arts: Array.isArray(nextState.arts) ? nextState.arts : [],
    };
}

function getSelectedArt() {
    if (!state.arts.length) {
        return null;
    }

    if (!state.arts.some((art) => art.id === selectedArtId)) {
        selectedArtId = state.selectedWeapon?.currentArtId ?? state.arts[0].id;
    }

    return state.arts.find((art) => art.id === selectedArtId) ?? state.arts[0];
}

function syncSelection() {
    if (!state.arts.length) {
        selectedArtId = null;
        return;
    }

    if (selectedArtId && state.arts.some((art) => art.id === selectedArtId)) {
        return;
    }

    if (
        state.selectedWeapon?.currentArtId &&
        state.arts.some((art) => art.id === state.selectedWeapon.currentArtId)
    ) {
        selectedArtId = state.selectedWeapon.currentArtId;
        return;
    }

    selectedArtId = state.arts[0].id;
}

function renderHeader() {
    elements.playerLevel.textContent = String(state.playerLevel);
    elements.playerPoints.textContent = String(state.playerPoint);
    elements.artCount.textContent = `${state.arts.length} Arts`;
}

function renderList() {
    if (!state.arts.length) {
        elements.artList.innerHTML = `
			<div class="empty-state">
				<div>
					<h3>No weapon arts loaded</h3>
					<p class="empty-copy">The menu is ready, but no weapon art data was received from the plugin yet.</p>
				</div>
			</div>`;
        return;
    }

    elements.artList.innerHTML = state.arts
        .map((art, index) => {
            const tags = [
                art.unlocked ? badge("Unlocked", "ok") : badge("Locked", "bad"),
                badge(`Lv ${art.unlockLevel}`, "warn"),
                badge(`Cost ${art.consumePoint}`, "warn"),
            ];

            if (state.selectedWeapon) {
                tags.push(
                    art.weaponAllowed
                        ? badge("Compatible", "ok")
                        : badge("Not compatible", "bad")
                );
            }

            if (art.isAssigned) {
                tags.push(badge("Assigned", "ok"));
            }

            return `
				<button
					class="art-item ${art.id === selectedArtId ? "active" : ""}"
					type="button"
					data-art-id="${art.id}"
					style="animation-delay:${index * 28}ms"
				>
					<div class="art-item-title">
						<strong>${art.name}</strong>
						<span class="art-item-id">${formatId(art.id)}</span>
					</div>
					<div class="art-item-meta">${tags.join("")}</div>
					<p class="art-item-copy">${art.description}</p>
				</button>`;
        })
        .join("");

    elements.artList.querySelectorAll("[data-art-id]").forEach((button) => {
        button.addEventListener("click", () => {
            selectedArtId = Number(button.dataset.artId);
            renderDetail();
            renderList();
        });
    });
}

function renderWeaponPanel() {
    if (!state.selectedWeapon) {
        elements.weaponPanel.innerHTML = `
			<div class="panel-title">
				<div>
					<p class="eyebrow">Current Weapon</p>
					<h2>No weapon selected</h2>
				</div>
				${badge("Inventory selection required", "bad")}
			</div>
			<div class="empty-state">
				<div>
					<h3>Select a weapon in the inventory</h3>
					<p class="empty-copy">The right panel will show the selected weapon, its current art, and whether the highlighted art can be assigned.</p>
				</div>
			</div>`;
        return;
    }

    elements.weaponPanel.innerHTML = `
		<div class="panel-title">
			<div>
				<p class="eyebrow">Current Weapon</p>
				<h2 class="weapon-name">${state.selectedWeapon.name}</h2>
			</div>
			${badge(state.selectedWeapon.currentArtName || "Unassigned", state.selectedWeapon.currentArtId ? "ok" : "warn")}
		</div>
		<div class="weapon-meta">
			${badge(state.selectedWeapon.type || "Unknown", "warn")}
			${badge(`Form ${state.selectedWeapon.formId}`, "warn")}
		</div>
		<p class="weapon-description">The highlighted weapon art can be bound to this weapon if it is unlocked and compatible.</p>`;
}

function renderDetail() {
    const art = getSelectedArt();

    if (!art) {
        elements.detailPanel.innerHTML = `
			<div class="panel-title">
				<div>
					<p class="eyebrow">Selection</p>
					<h2>No art selected</h2>
				</div>
			</div>
			<div class="empty-state">
				<div>
					<h3>Waiting for weapon art data</h3>
					<p class="empty-copy">Once the plugin sends the full weapon art catalog, the selected entry will appear here.</p>
				</div>
			</div>`;
        return;
    }

    const unlockBlocked =
        !art.unlocked &&
        (state.playerLevel < art.unlockLevel || state.playerPoint < art.consumePoint);
    const canAssign = Boolean(
        state.selectedWeapon && art.unlocked && art.weaponAllowed && !art.isAssigned
    );

    const statusBadges = [
        badge(art.unlocked ? "Unlocked" : "Locked", art.unlocked ? "ok" : "bad"),
        badge(`Lv ${art.unlockLevel}`, "warn"),
        badge(`Cost ${art.consumePoint}`, "warn"),
        badge(art.damageType || "None", "warn"),
    ];

    if (state.selectedWeapon) {
        statusBadges.push(
            art.weaponAllowed ? badge("Compatible", "ok") : badge("Not compatible", "bad")
        );
    }

    if (art.isAssigned) {
        statusBadges.push(badge("Currently assigned", "ok"));
    }

    elements.detailPanel.innerHTML = `
		<div class="panel-title">
			<div>
				<p class="eyebrow">Selected Art</p>
				<h2 class="detail-name">${art.name}</h2>
			</div>
			<span class="counter-pill">${formatId(art.id)}</span>
		</div>
		<div class="tag-row">${statusBadges.join("")}</div>
		<p class="detail-description">${art.description}</p>
		<div class="detail-grid">
			${metricCard("Damage Type", art.damageType || "None")}
			${metricCard("Damage Mult", Number(art.damageMult ?? 0).toFixed(2))}
			${metricCard("Base Damage", Number(art.baseDamage ?? 0).toFixed(1))}
			${metricCard("Posture Mult", Number(art.postureDamageMult ?? 0).toFixed(2))}
		</div>
		<div class="action-row">
			<button class="action-button primary" id="unlock-button" type="button" ${art.unlocked || unlockBlocked ? "disabled" : ""}>
				${art.unlocked ? "Already Unlocked" : `Unlock Art (${art.consumePoint} pt)`}
			</button>
			<button class="action-button secondary" id="assign-button" type="button" ${canAssign ? "" : "disabled"}>
				${art.isAssigned ? "Assigned to Weapon" : "Assign to Selected Weapon"}
			</button>
		</div>
		<p class="action-hint">${buildActionHint(art, unlockBlocked, canAssign)}</p>`;

    const unlockButton = document.getElementById("unlock-button");
    const assignButton = document.getElementById("assign-button");

    if (unlockButton) {
        unlockButton.addEventListener("click", () => {
            callPrismaListener("unlockWeaponArt", String(art.id));
        });
    }

    if (assignButton) {
        assignButton.addEventListener("click", () => {
            if (canAssign) {
                callPrismaListener("setWeaponArt", String(art.id));
            }
        });
    }
}

function buildActionHint(art, unlockBlocked, canAssign) {
    if (!art.unlocked && unlockBlocked) {
        return `Requires level ${art.unlockLevel} and ${art.consumePoint} available point(s).`;
    }

    if (!state.selectedWeapon) {
        return "Select a weapon in the inventory to enable weapon-specific assignment.";
    }

    if (!art.weaponAllowed) {
        return "The selected weapon does not meet this art's weapon requirements.";
    }

    if (art.isAssigned) {
        return "This weapon is already using the highlighted art.";
    }

    if (canAssign) {
        return "This art is ready to be assigned to the currently selected weapon.";
    }

    if (!art.unlocked) {
        return "Unlock this art first, then it can be assigned to the selected weapon.";
    }

    return "Weapon art state updated.";
}

function renderAll() {
    renderHeader();
    renderWeaponPanel();
    renderDetail();
    renderList();
}

window.setMenuState = function setMenuState(payload) {
    try {
        const parsed = JSON.parse(payload);
        const nextState = normalizeState(parsed);

        state.playerLevel = nextState.playerLevel;
        state.playerPoint = nextState.playerPoint;
        state.selectedWeapon = nextState.selectedWeapon;
        state.arts = nextState.arts;
        state.filterEnabled = nextState.filterEnabled;

        syncSelection();
        renderAll();
    } catch (error) {
        console.error("Failed to apply weapon art menu state", error, payload);
    }
};

elements.closeButton.addEventListener("click", () => {
    callPrismaListener("closeWeaponArtMenu", "");
});

window.addEventListener("keydown", (event) => {
    if (event.key === "Escape") {
        event.preventDefault();
        callPrismaListener("closeWeaponArtMenu", "");
    }
});

renderAll();
