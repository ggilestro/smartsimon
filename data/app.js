// Simon Says - Web Application
// Author: Giorgio Gilestro

// Configuration
const WS_RECONNECT_INTERVAL = 5000;
const API_BASE = window.location.origin;

// Global State
let ws = null;
let currentDifficulty = 0;
let wsReconnectTimer = null;

// Initialize Application
document.addEventListener('DOMContentLoaded', () => {
    console.log('Simon Says Web App starting...');

    syncTime();  // Sync ESP32 time with browser first
    initTabs();
    initControls();
    connectWebSocket();
    loadInitialData();
});

// ============================================================================
// Tab System
// ============================================================================

function initTabs() {
    const tabButtons = document.querySelectorAll('.tab');

    tabButtons.forEach(button => {
        button.addEventListener('click', () => {
            const tabName = button.dataset.tab;
            switchTab(tabName);
        });
    });
}

function switchTab(tabName) {
    // Update tab buttons
    document.querySelectorAll('.tab').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.tab === tabName);
    });

    // Update tab content
    document.querySelectorAll('.tab-content').forEach(content => {
        content.classList.toggle('active', content.id === `${tabName}-tab`);
    });

    // Load tab data
    switch(tabName) {
        case 'leaderboard':
            loadLeaderboard();
            break;
        case 'players':
            loadPlayers();
            break;
        case 'settings':
            loadSettings();
            break;
    }
}

// ============================================================================
// Controls Setup
// ============================================================================

function initControls() {
    // Game controls
    document.getElementById('startBtn').addEventListener('click', startGame);
    document.getElementById('stopBtn').addEventListener('click', stopGame);

    // Player selector
    document.getElementById('playerSelect').addEventListener('change', (e) => {
        setCurrentPlayer(e.target.value);
    });

    // Load players into dropdown
    loadPlayerSelector();

    // Difficulty tabs
    document.querySelectorAll('.difficulty-tab').forEach(btn => {
        btn.addEventListener('click', (e) => {
            const difficulty = parseInt(e.target.dataset.difficulty);
            selectDifficultyTab(difficulty);
            loadDifficultyScores(difficulty);
        });
    });

    // Player management
    document.getElementById('createPlayerBtn').addEventListener('click', createPlayer);
    document.getElementById('playerName').addEventListener('keypress', (e) => {
        if (e.key === 'Enter') createPlayer();
    });

    // Settings
    document.getElementById('volumeSlider').addEventListener('input', (e) => {
        document.getElementById('volumeValue').textContent = e.target.value;
    });

    document.getElementById('brightnessSlider').addEventListener('input', (e) => {
        document.getElementById('brightnessValue').textContent = e.target.value;
    });

    document.getElementById('saveSettingsBtn').addEventListener('click', saveSettings);
    document.getElementById('factoryResetBtn').addEventListener('click', factoryReset);

    // Multiplayer controls
    initMultiplayer();
}

// ============================================================================
// Multiplayer Functions
// ============================================================================

function initMultiplayer() {
    // Load player checkboxes
    loadMultiplayerPlayers();

    // Start button
    document.getElementById('startMultiplayerBtn').addEventListener('click', startMultiplayerGame);

    // Listen for checkbox changes to enable/disable start button
    document.getElementById('multiplayerPlayerList').addEventListener('change', validateMultiplayerSelection);
}

async function loadMultiplayerPlayers() {
    try {
        const response = await fetch(`${API_BASE}/api/players`);
        const players = await response.json();

        const container = document.getElementById('multiplayerPlayerList');
        container.innerHTML = '';

        if (players.length === 0) {
            container.innerHTML = '<p class="help-text">No players available. Create players in the Players tab first.</p>';
            return;
        }

        players.forEach(player => {
            const label = document.createElement('label');
            label.className = 'checkbox-label';
            label.innerHTML = `
                <input type="checkbox" name="multiplayerPlayer" value="${player.id}">
                <span>${player.name} (Best: ${player.bestScore}, Games: ${player.gamesPlayed})</span>
            `;
            container.appendChild(label);
        });

        validateMultiplayerSelection();
    } catch (error) {
        console.error('Error loading multiplayer players:', error);
        showToast('Failed to load players', 'error');
    }
}

function validateMultiplayerSelection() {
    const checkboxes = document.querySelectorAll('input[name="multiplayerPlayer"]:checked');
    const startBtn = document.getElementById('startMultiplayerBtn');
    const count = checkboxes.length;

    startBtn.disabled = (count < 2 || count > 4);

    if (count < 2) {
        startBtn.textContent = 'Select 2-4 Players';
    } else if (count > 4) {
        startBtn.textContent = 'Max 4 Players';
    } else {
        startBtn.textContent = `Start Multiplayer Game (${count} players)`;
    }
}

async function startMultiplayerGame() {
    const checkboxes = document.querySelectorAll('input[name="multiplayerPlayer"]:checked');
    const playerIds = Array.from(checkboxes).map(cb => cb.value);

    if (playerIds.length < 2 || playerIds.length > 4) {
        showToast('Please select 2-4 players', 'error');
        return;
    }

    const difficulty = parseInt(document.getElementById('multiplayerDifficulty').value);

    try {
        const response = await fetch(`${API_BASE}/api/game/multiplayer/start`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                difficulty: difficulty,
                playerIds: playerIds
            })
        });

        if (response.ok) {
            showToast('Multiplayer game started!', 'success');

            // Show multiplayer status card
            document.getElementById('multiplayerStatus').style.display = 'block';
        } else {
            const error = await response.text();
            showToast(`Failed to start game: ${error}`, 'error');
        }
    } catch (error) {
        console.error('Error starting multiplayer game:', error);
        showToast('Failed to start multiplayer game', 'error');
    }
}

function handleMultiplayerUpdate(data) {
    const currentPlayerEl = document.getElementById('multiCurrentPlayer');
    const currentScoreEl = document.getElementById('multiCurrentScore');
    const scoresEl = document.getElementById('multiplayerScores');

    // Update current player
    if (currentPlayerEl && data.currentPlayerName) {
        currentPlayerEl.textContent = data.currentPlayerName;
    }

    // Update current score (from game state message)
    if (currentScoreEl) {
        const score = document.getElementById('currentScore').textContent;
        currentScoreEl.textContent = score;
    }

    // Update all player scores
    if (scoresEl && data.players) {
        let html = '<div class="player-scores">';
        data.players.forEach((player, index) => {
            const statusClass = player.hasPlayed ? 'eliminated' : (index === data.currentPlayerIndex ? 'active' : '');
            const statusText = player.hasPlayed ? '✓ Finished' : (index === data.currentPlayerIndex ? '▶️ Playing' : '⏸️ Waiting');

            html += `
                <div class="score-item ${statusClass}">
                    <span>${player.name}</span>
                    <span>${player.score} - ${statusText}</span>
                </div>
            `;
        });
        html += '</div>';
        scoresEl.innerHTML = html;
    }
}

// ============================================================================
// WebSocket Connection
// ============================================================================

function connectWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws`;

    console.log('Connecting to WebSocket:', wsUrl);
    updateConnectionStatus('connecting');

    ws = new WebSocket(wsUrl);

    ws.onopen = () => {
        console.log('WebSocket connected');
        updateConnectionStatus('connected');
        if (wsReconnectTimer) {
            clearTimeout(wsReconnectTimer);
            wsReconnectTimer = null;
        }
    };

    ws.onclose = () => {
        console.log('WebSocket disconnected');
        updateConnectionStatus('disconnected');
        scheduleReconnect();
    };

    ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        updateConnectionStatus('disconnected');
    };

    ws.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            handleWebSocketMessage(data);
        } catch (e) {
            console.error('Failed to parse WebSocket message:', e);
        }
    };
}

function scheduleReconnect() {
    if (!wsReconnectTimer) {
        wsReconnectTimer = setTimeout(() => {
            console.log('Attempting to reconnect...');
            connectWebSocket();
        }, WS_RECONNECT_INTERVAL);
    }
}

function updateConnectionStatus(status) {
    const statusEl = document.getElementById('connectionStatus');
    const dot = statusEl.querySelector('.status-dot');
    const text = statusEl.querySelector('.status-text');
    const overlay = document.getElementById('connectionOverlay');

    dot.className = 'status-dot';

    switch(status) {
        case 'connected':
            dot.classList.add('connected');
            text.textContent = 'Connected';
            // Hide overlay when connected
            overlay.classList.add('hidden');
            break;
        case 'disconnected':
            dot.classList.add('disconnected');
            text.textContent = 'Disconnected';
            // Show overlay when disconnected
            overlay.classList.remove('hidden');
            overlay.querySelector('h2').textContent = 'Connection Lost';
            overlay.querySelector('p').textContent = 'Reconnecting to device...';
            break;
        case 'connecting':
            text.textContent = 'Connecting...';
            // Show overlay when connecting
            overlay.classList.remove('hidden');
            overlay.querySelector('h2').textContent = 'Connecting to Device...';
            overlay.querySelector('p').textContent = 'Please wait while we establish connection';
            break;
    }
}

function handleWebSocketMessage(data) {
    console.log('WS Message:', data);

    switch(data.type) {
        case 'gameState':
            updateGameState(data);
            break;
        case 'sequence':
            animateSequence(data.colors);
            break;
        case 'buttonPress':
            flashButton(data.color, data.correct);
            break;
        case 'gameOver':
            showGameOver(data);
            break;
        case 'multiplayer':
            handleMultiplayerUpdate(data);
            break;
    }
}

// ============================================================================
// Game State Updates
// ============================================================================

function updateGameState(data) {
    document.getElementById('gameState').textContent = data.state || 'IDLE';
    document.getElementById('currentScore').textContent = data.score || 0;
    document.getElementById('highScore').textContent = data.highScore || 0;
    document.getElementById('currentDifficulty').textContent = data.difficulty || 'Easy';
}

function animateSequence(colors) {
    colors.forEach((color, index) => {
        setTimeout(() => {
            flashButton(color.toLowerCase(), true);
        }, index * 800);
    });
}

function flashButton(color, correct) {
    const btn = document.querySelector(`.simon-btn[data-color="${color}"]`);
    if (btn) {
        btn.classList.add('active');
        setTimeout(() => btn.classList.remove('active'), 300);
    }
}

function showGameOver(data) {
    const message = data.highScore ?
        `🎉 New High Score: ${data.score}!` :
        `Game Over! Score: ${data.score}`;
    showToast(message, data.highScore ? 'success' : 'error');
}

// ============================================================================
// Game Control
// ============================================================================

async function startGame() {
    const difficulty = parseInt(document.getElementById('difficulty').value);

    try {
        const response = await fetch(`${API_BASE}/api/game/start`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ difficulty })
        });

        if (response.ok) {
            document.getElementById('startBtn').disabled = true;
            document.getElementById('stopBtn').disabled = false;
            showToast('Game started!', 'success');
        }
    } catch (error) {
        console.error('Failed to start game:', error);
        showToast('Failed to start game', 'error');
    }
}

async function stopGame() {
    try {
        const response = await fetch(`${API_BASE}/api/game/stop`, {
            method: 'POST'
        });

        if (response.ok) {
            document.getElementById('startBtn').disabled = false;
            document.getElementById('stopBtn').disabled = true;
            showToast('Game stopped', 'success');
        }
    } catch (error) {
        console.error('Failed to stop game:', error);
        showToast('Failed to stop game', 'error');
    }
}

// ============================================================================
// Leaderboard
// ============================================================================

async function loadLeaderboard() {
    await Promise.all([
        loadAllTimeScores(),
        loadDifficultyScores(0),
        loadRecentGames()
    ]);
}

async function loadAllTimeScores() {
    try {
        const response = await fetch(`${API_BASE}/api/scores/high`);
        const scores = await response.json();

        const html = scores.length > 0 ?
            scores.map((s, i) => `
                <div class="score-item">
                    <div>
                        <div class="score-name">${i + 1}. ${s.playerName}</div>
                        <div class="score-meta">${s.difficulty} • ${formatDate(s.timestamp)}</div>
                    </div>
                    <div class="score-value">${s.score}</div>
                </div>
            `).join('') :
            '<p class="loading">No scores yet</p>';

        document.getElementById('allTimeScores').innerHTML = html;
    } catch (error) {
        console.error('Failed to load all-time scores:', error);
    }
}

async function loadDifficultyScores(difficulty) {
    try {
        const response = await fetch(`${API_BASE}/api/scores/difficulty/${difficulty}`);
        const scores = await response.json();

        const html = scores.length > 0 ?
            scores.map((s, i) => `
                <div class="score-item">
                    <div>
                        <div class="score-name">${i + 1}. ${s.playerName}</div>
                        <div class="score-meta">${formatDate(s.timestamp)}</div>
                    </div>
                    <div class="score-value">${s.score}</div>
                </div>
            `).join('') :
            '<p class="loading">No scores yet</p>';

        document.getElementById('difficultyScores').innerHTML = html;
    } catch (error) {
        console.error('Failed to load difficulty scores:', error);
    }
}

async function loadRecentGames() {
    try {
        const response = await fetch(`${API_BASE}/api/scores/recent`);
        const games = await response.json();

        const html = games.length > 0 ?
            games.map(g => `
                <div class="score-item">
                    <div>
                        <div class="score-name">${g.playerName}</div>
                        <div class="score-meta">${g.difficulty} • ${formatDate(g.timestamp)}</div>
                    </div>
                    <div class="score-value">${g.score}</div>
                </div>
            `).join('') :
            '<p class="loading">No games yet</p>';

        document.getElementById('recentGames').innerHTML = html;
    } catch (error) {
        console.error('Failed to load recent games:', error);
    }
}

function selectDifficultyTab(difficulty) {
    document.querySelectorAll('.difficulty-tab').forEach(btn => {
        btn.classList.toggle('active', parseInt(btn.dataset.difficulty) === difficulty);
    });
}

// ============================================================================
// Players
// ============================================================================

async function loadPlayerSelector() {
    try {
        const response = await fetch(`${API_BASE}/api/players`);
        const players = await response.json();

        const select = document.getElementById('playerSelect');

        // Clear existing options except guest
        select.innerHTML = '<option value="">Guest (no stats)</option>';

        // Add all players
        players.forEach(p => {
            const option = document.createElement('option');
            option.value = p.id;
            option.textContent = `${p.name} (${p.gamesPlayed} games, best: ${p.bestScore})`;
            select.appendChild(option);
        });

        console.log(`Loaded ${players.length} players into selector`);
    } catch (error) {
        console.error('Failed to load player selector:', error);
    }
}

async function setCurrentPlayer(playerId) {
    try {
        const response = await fetch(`${API_BASE}/api/game/player`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ playerId })
        });

        if (response.ok) {
            const data = await response.json();
            const playerName = playerId ?
                document.getElementById('playerSelect').selectedOptions[0].textContent :
                'Guest';
            showToast(`Current player: ${playerName}`, 'success');
        } else {
            showToast('Failed to set player', 'error');
        }
    } catch (error) {
        console.error('Failed to set current player:', error);
        showToast('Failed to set player', 'error');
    }
}

async function loadPlayers() {
    try {
        const response = await fetch(`${API_BASE}/api/players`);
        const players = await response.json();

        const html = players.length > 0 ?
            players.map(p => `
                <div class="player-item">
                    <div class="player-name">${p.name}</div>
                    <div class="player-stats">
                        <div class="player-stat">
                            <span class="player-stat-label">Games</span>
                            <span class="player-stat-value">${p.gamesPlayed}</span>
                        </div>
                        <div class="player-stat">
                            <span class="player-stat-label">Best</span>
                            <span class="player-stat-value">${p.bestScore}</span>
                        </div>
                        <div class="player-stat">
                            <span class="player-stat-label">Avg</span>
                            <span class="player-stat-value">${p.avgScore.toFixed(1)}</span>
                        </div>
                    </div>
                </div>
            `).join('') :
            '<p class="loading">No players yet. Create one above!</p>';

        document.getElementById('playersList').innerHTML = html;
    } catch (error) {
        console.error('Failed to load players:', error);
    }
}

async function createPlayer() {
    const nameInput = document.getElementById('playerName');
    const name = nameInput.value.trim();

    if (!name) {
        showToast('Please enter a name', 'error');
        return;
    }

    try {
        const response = await fetch(`${API_BASE}/api/players`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ name })
        });

        if (response.ok) {
            nameInput.value = '';
            showToast(`Player "${name}" created!`, 'success');
            loadPlayers();
            loadPlayerSelector();  // Refresh the dropdown
        } else {
            showToast('Failed to create player', 'error');
        }
    } catch (error) {
        console.error('Failed to create player:', error);
        showToast('Failed to create player', 'error');
    }
}

// ============================================================================
// Settings
// ============================================================================

async function loadSettings() {
    try {
        const response = await fetch(`${API_BASE}/api/settings`);
        const settings = await response.json();

        document.getElementById('volumeSlider').value = settings.volume;
        document.getElementById('volumeValue').textContent = settings.volume;

        document.getElementById('brightnessSlider').value = settings.ledBrightness;
        document.getElementById('brightnessValue').textContent = settings.ledBrightness;

        document.getElementById('soundEnabled').checked = settings.soundEnabled;
        document.getElementById('deepSleepEnabled').checked = settings.deepSleepEnabled;

        // Load storage stats
        const storageResponse = await fetch(`${API_BASE}/api/storage`);
        const storage = await storageResponse.json();

        const html = `
            <p><strong>Total:</strong> ${formatBytes(storage.totalBytes)}</p>
            <p><strong>Used:</strong> ${formatBytes(storage.usedBytes)} (${storage.usedPercent.toFixed(1)}%)</p>
            <div class="storage-bar">
                <div class="storage-fill" style="width: ${storage.usedPercent}%"></div>
            </div>
        `;

        document.getElementById('storageInfo').innerHTML = html;
    } catch (error) {
        console.error('Failed to load settings:', error);
    }
}

async function saveSettings() {
    const settings = {
        difficulty: parseInt(document.getElementById('difficulty').value),
        volume: parseInt(document.getElementById('volumeSlider').value),
        ledBrightness: parseInt(document.getElementById('brightnessSlider').value),
        soundEnabled: document.getElementById('soundEnabled').checked,
        deepSleepEnabled: document.getElementById('deepSleepEnabled').checked
    };

    try {
        const response = await fetch(`${API_BASE}/api/settings`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(settings)
        });

        if (response.ok) {
            showToast('Settings saved!', 'success');
        } else {
            showToast('Failed to save settings', 'error');
        }
    } catch (error) {
        console.error('Failed to save settings:', error);
        showToast('Failed to save settings', 'error');
    }
}

async function factoryReset() {
    if (!confirm('Are you sure? This will delete ALL data (players, scores, settings)!')) {
        return;
    }

    try {
        const response = await fetch(`${API_BASE}/api/reset`, {
            method: 'POST'
        });

        if (response.ok) {
            showToast('Factory reset complete!', 'success');
            setTimeout(() => window.location.reload(), 2000);
        } else {
            showToast('Failed to reset', 'error');
        }
    } catch (error) {
        console.error('Failed to factory reset:', error);
        showToast('Failed to reset', 'error');
    }
}

// ============================================================================
// Time Synchronization
// ============================================================================

async function syncTime() {
    try {
        const timestamp = Math.floor(Date.now() / 1000);  // Unix timestamp in seconds

        const response = await fetch(`${API_BASE}/api/time`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ timestamp })
        });

        if (response.ok) {
            console.log('Time synchronized with ESP32:', new Date(timestamp * 1000));
        } else {
            console.error('Failed to sync time with ESP32');
        }
    } catch (error) {
        console.error('Time sync error:', error);
    }
}

// ============================================================================
// Initial Data Load
// ============================================================================

async function loadInitialData() {
    try {
        const response = await fetch(`${API_BASE}/api/game/status`);
        const status = await response.json();
        updateGameState(status);
    } catch (error) {
        console.error('Failed to load game status:', error);
    }
}

// ============================================================================
// Utility Functions
// ============================================================================

function showToast(message, type = 'success') {
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.textContent = message;

    document.body.appendChild(toast);

    setTimeout(() => {
        toast.remove();
    }, 3000);
}

function formatDate(timestamp) {
    const date = new Date(timestamp * 1000);
    return date.toLocaleDateString() + ' ' + date.toLocaleTimeString();
}

function formatBytes(bytes) {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
}
