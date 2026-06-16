const state = {
    playerLevel: 1,
    playerPoint: 0,
    selectedWeapon: null,
    arts: [],
    filterEnabled: false,
};

const strings = {
    brand: "RimCombat",
    title: "Weapon Arts",
    level: "Weapon Art Level",
    points: "Weapon Art Points",
    close: "Close",
    catalog: "Catalog",
    catalogTitle: "All Weapon Arts",
    catalogNote: "Select a weapon art below. The upper panels show the selected weapon and the highlighted art.",
    artCount: "{} Arts",
    noDescription: "No description available.",
    noArtsTitle: "No weapon arts loaded",
    noArtsBody: "The menu is ready, but no weapon art data was received from the plugin yet.",
    currentWeapon: "Current Weapon",
    noWeaponSelected: "No weapon selected",
    inventorySelectionRequired: "Inventory selection required",
    selectWeaponTitle: "Select a weapon in the inventory",
    selectWeaponBody: "The upper-left panel shows the selected weapon, its equipped art, and the bind state.",
    boundArt: "Bound Art",
    weaponType: "Weapon Type",
    unassigned: "Unassigned",
    unknown: "Unknown",
    weaponBody: "The highlighted weapon art can be bound to this weapon if it is unlocked and compatible.",
    noWeaponArtSelected: "No Weapon Art Selected",
    selectedArtAlreadyBound: "Selected Art Already Bound",
    bindAction: "Bind {}",
    unbindCurrent: "Unbind Current Weapon Art",
    noWeaponArtBound: "No Weapon Art Bound",
    selectedArt: "Selected Art",
    noArtSelected: "No art selected",
    waitingTitle: "Waiting for weapon art data",
    waitingBody: "Once the plugin sends the catalog, the selected entry will appear here.",
    unlockLevel: "Unlock Level",
    pointCost: "Point Cost",
    activation: "Activation",
    assignment: "Assignment",
    available: "Available",
    currentlyAssigned: "Currently Assigned",
    unlocked: "Unlocked",
    locked: "Locked",
    levelBadge: "Lv {}",
    costBadge: "Cost {}",
    prepare: "Prepared",
    instant: "Instant",
    compatible: "Compatible",
    incompatible: "Not compatible",
    assigned: "Assigned",
    alreadyUnlocked: "Already Unlocked",
    unlockAction: "Unlock Art ({} pt)",
    assignedToWeapon: "Assigned to Weapon",
    assignToSelectedWeapon: "Assign to Selected Weapon",
    hintSelectArt: "Select a weapon art from the grid to bind it to this weapon.",
    hintCanBind: "Bind {} to this weapon, or clear the current binding.",
    hintHasBinding: "This weapon already has a bound weapon art. You can replace it with the highlighted compatible art or clear it.",
    hintNoBinding: "Select an unlocked compatible weapon art to bind it, or leave this weapon unassigned.",
    hintUnlockBlocked: "Requires level {} and {} available point(s).",
    hintNeedWeapon: "Select a weapon in the inventory to enable weapon-specific assignment.",
    hintIncompatible: "The selected weapon does not meet this art's weapon requirements.",
    hintAlreadyAssigned: "This weapon is already using the highlighted art.",
    hintReadyToAssign: "This art is ready to be assigned to the currently selected weapon.",
    hintUnlockFirst: "Unlock this art first, then it can be assigned to the selected weapon.",
    hintStateUpdated: "Weapon art state updated.",
};

const elements = {
    artList: document.getElementById("art-list"),
    artCount: document.getElementById("art-count"),
    playerLevel: document.getElementById("player-level"),
    playerPoints: document.getElementById("player-points"),
    weaponPanel: document.getElementById("weapon-panel"),
    detailPanel: document.getElementById("detail-panel"),
    closeButton: document.getElementById("close-button"),
    menuBrand: document.getElementById("menu-brand"),
    menuTitle: document.getElementById("menu-title"),
    levelLabel: document.getElementById("level-label"),
    pointsLabel: document.getElementById("points-label"),
    catalogKicker: document.getElementById("catalog-kicker"),
    catalogTitle: document.getElementById("catalog-title"),
    catalogNote: document.getElementById("catalog-note"),
};

let selectedArtId = null;

function formatText(template, ...values) {
    let result = String(template ?? "");
    values.forEach((value) => {
        result = result.replace("{}", String(value ?? ""));
    });
    return result;
}

function escapeHtml(value) {
    return String(value ?? "")
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#39;");
}

function describe(value) {
    const text = String(value ?? "").trim();
    return text || strings.noDescription;
}

function callPrismaListener(name, payload = "") {
    const listener = window[name];
    if (typeof listener === "function") {
        listener(payload);
    } else {
        console.info(`[WeaponArtMenu] Listener unavailable: ${name}`, payload);
    }
}

function badge(label, tone = "warn") {
    return `<span class="badge ${tone}">${escapeHtml(label)}</span>`;
}

function infoCard(label, value) {
    return `<article class="info-card"><span>${escapeHtml(label)}</span><strong>${escapeHtml(value)}</strong></article>`;
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

function applyStaticText() {
    elements.menuBrand.textContent = strings.brand;
    elements.menuTitle.textContent = strings.title;
    elements.levelLabel.textContent = strings.level;
    elements.pointsLabel.textContent = strings.points;
    elements.closeButton.textContent = strings.close;
    elements.catalogKicker.textContent = strings.catalog;
    elements.catalogTitle.textContent = strings.catalogTitle;
    elements.catalogNote.textContent = strings.catalogNote;
}

function renderHeader() {
    elements.playerLevel.textContent = String(state.playerLevel);
    elements.playerPoints.textContent = String(state.playerPoint);
    elements.artCount.textContent = formatText(strings.artCount, state.arts.length);
}

function renderList() {
    if (!state.arts.length) {
        elements.artList.innerHTML = `
            <div class="empty-state">
                <div>
                    <h3>${escapeHtml(strings.noArtsTitle)}</h3>
                    <p class="empty-copy">${escapeHtml(strings.noArtsBody)}</p>
                </div>
            </div>`;
        return;
    }

    elements.artList.innerHTML = state.arts
        .map((art, index) => {
            const tags = [
                art.unlocked ? badge(strings.unlocked, "ok") : badge(strings.locked, "bad"),
                badge(formatText(strings.levelBadge, art.unlockLevel), "warn"),
                badge(formatText(strings.costBadge, art.consumePoint), "warn"),
                badge(art.needPrepare ? strings.prepare : strings.instant, "warn"),
            ];

            if (state.selectedWeapon) {
                tags.push(
                    art.weaponAllowed
                        ? badge(strings.compatible, "ok")
                        : badge(strings.incompatible, "bad")
                );
            }

            if (art.isAssigned) {
                tags.push(badge(strings.assigned, "ok"));
            }

            return `
                <button
                    class="art-card ${art.id === selectedArtId ? "active" : ""}"
                    type="button"
                    data-art-id="${art.id}"
                    style="animation-delay:${index * 24}ms"
                >
                    <div class="art-card-top">
                        <div>
                            <p class="section-kicker">${escapeHtml(art.needPrepare ? strings.prepare : strings.instant)}</p>
                            <h3 class="art-card-name">${escapeHtml(art.name)}</h3>
                        </div>
                        <span class="card-index">${String(index + 1).padStart(2, "0")}</span>
                    </div>
                    <div class="badge-row">${tags.join("")}</div>
                    <p class="art-card-copy">${escapeHtml(describe(art.description))}</p>
                </button>`;
        })
        .join("");

    elements.artList.querySelectorAll("[data-art-id]").forEach((button) => {
        button.addEventListener("click", () => {
            selectedArtId = Number(button.dataset.artId);
            renderAll();
        });
    });
}

function renderWeaponPanel() {
    if (!state.selectedWeapon) {
        elements.weaponPanel.innerHTML = `
            <div class="panel-header">
                <div>
                    <p class="section-kicker">${escapeHtml(strings.currentWeapon)}</p>
                    <h2 class="panel-name">${escapeHtml(strings.noWeaponSelected)}</h2>
                </div>
                ${badge(strings.inventorySelectionRequired, "bad")}
            </div>
            <div class="empty-state">
                <div>
                    <h3>${escapeHtml(strings.selectWeaponTitle)}</h3>
                    <p class="empty-copy">${escapeHtml(strings.selectWeaponBody)}</p>
                </div>
            </div>`;
        return;
    }

    const art = getSelectedArt();
    const currentArtName = state.selectedWeapon.currentArtName || strings.unassigned;
    const weaponType = state.selectedWeapon.type || strings.unknown;
    const hasAssignedArt = Boolean(state.selectedWeapon.currentArtId);
    const canAssign = Boolean(
        art && state.selectedWeapon && art.unlocked && art.weaponAllowed && !art.isAssigned
    );
    const assignLabel = !art
        ? strings.noWeaponArtSelected
        : art.isAssigned
            ? strings.selectedArtAlreadyBound
            : formatText(strings.bindAction, art.name);
    const weaponHint = !art
        ? strings.hintSelectArt
        : canAssign
            ? formatText(strings.hintCanBind, art.name)
            : hasAssignedArt
                ? strings.hintHasBinding
                : strings.hintNoBinding;

    const weaponBadges = [badge(weaponType, "warn")];
    if (art) {
        weaponBadges.push(art.weaponAllowed ? badge(strings.compatible, "ok") : badge(strings.incompatible, "bad"));
    }

    elements.weaponPanel.innerHTML = `
        <div class="panel-header">
            <div>
                <p class="section-kicker">${escapeHtml(strings.currentWeapon)}</p>
                <h2 class="panel-name">${escapeHtml(state.selectedWeapon.name)}</h2>
            </div>
            ${badge(currentArtName, hasAssignedArt ? "ok" : "warn")}
        </div>
        <div class="badge-row">${weaponBadges.join("")}</div>
        <div class="info-grid">
            ${infoCard(strings.boundArt, currentArtName)}
            ${infoCard(strings.weaponType, weaponType)}
        </div>
        <p class="panel-copy">${escapeHtml(strings.weaponBody)}</p>
        <div class="action-row">
            <button class="action-button primary" id="weapon-assign-button" type="button" ${canAssign ? "" : "disabled"}>
                ${escapeHtml(assignLabel)}
            </button>
            <button class="action-button secondary" id="unbind-button" type="button" ${hasAssignedArt ? "" : "disabled"}>
                ${escapeHtml(hasAssignedArt ? strings.unbindCurrent : strings.noWeaponArtBound)}
            </button>
        </div>
        <p class="hint-line">${escapeHtml(weaponHint)}</p>`;

    const weaponAssignButton = document.getElementById("weapon-assign-button");
    if (weaponAssignButton) {
        weaponAssignButton.addEventListener("click", () => {
            if (canAssign && art) {
                callPrismaListener("setWeaponArt", String(art.id));
            }
        });
    }

    const unbindButton = document.getElementById("unbind-button");
    if (unbindButton) {
        unbindButton.addEventListener("click", () => {
            if (hasAssignedArt) {
                callPrismaListener("setWeaponArt", "0");
            }
        });
    }
}

function renderDetail() {
    const art = getSelectedArt();

    if (!art) {
        elements.detailPanel.innerHTML = `
            <div class="panel-header">
                <div>
                    <p class="section-kicker">${escapeHtml(strings.selectedArt)}</p>
                    <h2 class="panel-name">${escapeHtml(strings.noArtSelected)}</h2>
                </div>
            </div>
            <div class="empty-state">
                <div>
                    <h3>${escapeHtml(strings.waitingTitle)}</h3>
                    <p class="empty-copy">${escapeHtml(strings.waitingBody)}</p>
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
        badge(art.unlocked ? strings.unlocked : strings.locked, art.unlocked ? "ok" : "bad"),
        badge(formatText(strings.levelBadge, art.unlockLevel), "warn"),
        badge(formatText(strings.costBadge, art.consumePoint), "warn"),
        badge(art.needPrepare ? strings.prepare : strings.instant, "warn"),
    ];

    if (state.selectedWeapon) {
        statusBadges.push(art.weaponAllowed ? badge(strings.compatible, "ok") : badge(strings.incompatible, "bad"));
    }

    if (art.isAssigned) {
        statusBadges.push(badge(strings.currentlyAssigned, "ok"));
    }

    elements.detailPanel.innerHTML = `
        <div class="panel-header">
            <div>
                <p class="section-kicker">${escapeHtml(strings.selectedArt)}</p>
                <h2 class="panel-name">${escapeHtml(art.name)}</h2>
            </div>
        </div>
        <div class="badge-row">${statusBadges.join("")}</div>
        <p class="panel-copy">${escapeHtml(describe(art.description))}</p>
        <div class="info-grid">
            ${infoCard(strings.unlockLevel, art.unlockLevel)}
            ${infoCard(strings.pointCost, art.consumePoint)}
            ${infoCard(strings.activation, art.needPrepare ? strings.prepare : strings.instant)}
            ${infoCard(strings.assignment, art.isAssigned ? strings.assigned : strings.available)}
        </div>
        <div class="action-row">
            <button class="action-button primary" id="unlock-button" type="button" ${art.unlocked || unlockBlocked ? "disabled" : ""}>
                ${escapeHtml(art.unlocked ? strings.alreadyUnlocked : formatText(strings.unlockAction, art.consumePoint))}
            </button>
            <button class="action-button secondary" id="assign-button" type="button" ${canAssign ? "" : "disabled"}>
                ${escapeHtml(art.isAssigned ? strings.assignedToWeapon : strings.assignToSelectedWeapon)}
            </button>
        </div>
        <p class="hint-line">${escapeHtml(buildActionHint(art, unlockBlocked, canAssign))}</p>`;

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
        return formatText(strings.hintUnlockBlocked, art.unlockLevel, art.consumePoint);
    }

    if (!state.selectedWeapon) {
        return strings.hintNeedWeapon;
    }

    if (!art.weaponAllowed) {
        return strings.hintIncompatible;
    }

    if (art.isAssigned) {
        return strings.hintAlreadyAssigned;
    }

    if (canAssign) {
        return strings.hintReadyToAssign;
    }

    if (!art.unlocked) {
        return strings.hintUnlockFirst;
    }

    return strings.hintStateUpdated;
}

function renderAll() {
    applyStaticText();
    renderHeader();
    renderWeaponPanel();
    renderDetail();
    renderList();
}

window.setMenuConfig = function setMenuConfig(payload) {
    try {
        const data = JSON.parse(payload);
        if (typeof data.startPercent === "number") {
            document.documentElement.style.setProperty("--menu-start", `${data.startPercent}%`);
        }
        if (data.strings && typeof data.strings === "object") {
            Object.assign(strings, data.strings);
        }
        renderAll();
    } catch (error) {
        console.error("Failed to apply weapon art menu config", error, payload);
    }
};

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
