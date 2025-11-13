/*
 * Hoflix Utility Functions
 *
 * Shared JavaScript extracted from login.html, gallery.html, player.html
 * Reduces duplication and centralizes common functionality
 */

// ============================================================================
// Time Formatting Utilities
// ============================================================================

/**
 * Format duration from seconds to MM:SS
 * @param {number} seconds - Duration in seconds
 * @returns {string} Formatted time string (e.g., "5:30")
 */
function formatTime(seconds) {
    if (!seconds || isNaN(seconds) || seconds === 0) return '0:00';
    const mins = Math.floor(seconds / 60);
    const secs = Math.floor(seconds % 60);
    return `${mins}:${String(secs).padStart(2, '0')}`;
}

/**
 * Alias for formatTime (for backwards compatibility)
 * @param {number} seconds - Duration in seconds
 * @returns {string} Formatted time string
 */
function formatDuration(seconds) {
    return formatTime(seconds);
}

// ============================================================================
// Navbar Scroll Effect (Gallery & Player Pages)
// ============================================================================

/**
 * Initialize navbar scroll effect
 * Adds 'scrolled' class to navbar when user scrolls down
 * @param {string} navbarId - ID of the navbar element (default: 'navbar')
 * @param {number} threshold - Scroll threshold in pixels (default: 50)
 */
function initNavbarScroll(navbarId = 'navbar', threshold = 50) {
    window.addEventListener('scroll', () => {
        const navbar = document.getElementById(navbarId);
        if (!navbar) return;

        if (window.scrollY > threshold) {
            navbar.classList.add('scrolled');
        } else {
            navbar.classList.remove('scrolled');
        }
    });
}

// ============================================================================
// Modal Control Utilities
// ============================================================================

/**
 * Initialize modal controls
 * Handles open, close, and outside-click-to-close behavior
 * @param {string} modalId - ID of the modal element
 * @param {string} triggerSelector - Selector for elements that open the modal
 * @param {string} closeSelector - Selector for close button (default: '.close')
 * @param {Function} onOpen - Optional callback when modal opens
 * @param {Function} onClose - Optional callback when modal closes
 */
function initModal(modalId, triggerSelector, closeSelector = '.close', onOpen = null, onClose = null) {
    const modal = document.getElementById(modalId);
    const triggers = document.querySelectorAll(triggerSelector);
    const closeBtn = modal ? modal.querySelector(closeSelector) : null;

    if (!modal) {
        console.warn(`Modal with ID "${modalId}" not found`);
        return;
    }

    // Open modal when trigger is clicked
    triggers.forEach(trigger => {
        trigger.addEventListener('click', (e) => {
            e.preventDefault();
            modal.style.display = 'flex';
            if (onOpen) onOpen(modal);
        });
    });

    // Close modal when close button is clicked
    if (closeBtn) {
        closeBtn.addEventListener('click', () => {
            modal.style.display = 'none';
            if (onClose) onClose(modal);
        });
    }

    // Close modal when clicking outside
    window.addEventListener('click', (e) => {
        if (e.target === modal) {
            modal.style.display = 'none';
            if (onClose) onClose(modal);
        }
    });
}

// ============================================================================
// Form Validation Utilities
// ============================================================================

/**
 * Validate username format
 * @param {string} username - Username to validate
 * @returns {Object} {valid: boolean, message: string}
 */
function validateUsername(username) {
    const trimmed = username.trim();

    if (trimmed.length === 0) {
        return { valid: false, message: '사용자 이름을 입력해주세요' };
    }

    if (trimmed.length < 2) {
        return { valid: false, message: '사용자 이름은 최소 2글자 이상이어야 합니다' };
    }

    if (trimmed.length > 63) {
        return { valid: false, message: '사용자 이름은 최대 63글자까지 가능합니다' };
    }

    if (!/^[a-zA-Z0-9_]+$/.test(trimmed)) {
        return { valid: false, message: '사용자 이름은 영문, 숫자, 언더스코어만 가능합니다' };
    }

    return { valid: true, message: '' };
}

/**
 * Validate password strength
 * @param {string} password - Password to validate
 * @returns {Object} {valid: boolean, message: string}
 */
function validatePassword(password) {
    if (password.length === 0) {
        return { valid: false, message: '비밀번호를 입력해주세요' };
    }

    if (password.length < 8) {
        return { valid: false, message: '비밀번호는 최소 8자 이상이어야 합니다' };
    }

    const hasLetter = /[a-zA-Z]/.test(password);
    const hasNumber = /[0-9]/.test(password);

    if (!hasLetter || !hasNumber) {
        return { valid: false, message: '비밀번호는 영문자와 숫자를 모두 포함해야 합니다' };
    }

    return { valid: true, message: '' };
}

/**
 * Validate password confirmation
 * @param {string} password - Original password
 * @param {string} confirm - Confirmation password
 * @returns {Object} {valid: boolean, message: string}
 */
function validatePasswordMatch(password, confirm) {
    if (password !== confirm) {
        return { valid: false, message: '비밀번호가 일치하지 않습니다' };
    }
    return { valid: true, message: '' };
}

// ============================================================================
// API Request Utilities
// ============================================================================

/**
 * Make an authenticated API request
 * @param {string} url - API endpoint URL
 * @param {Object} options - Fetch options (method, headers, body, etc.)
 * @returns {Promise<Object>} Response data or throws error
 */
async function apiRequest(url, options = {}) {
    try {
        const defaultOptions = {
            credentials: 'include', // Include cookies for session
            headers: {
                'Content-Type': 'application/json',
                ...options.headers
            }
        };

        const response = await fetch(url, { ...defaultOptions, ...options });
        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.message || `HTTP ${response.status}: ${response.statusText}`);
        }

        return data;
    } catch (error) {
        console.error(`API request failed: ${url}`, error);
        throw error;
    }
}

/**
 * POST request helper
 * @param {string} url - API endpoint URL
 * @param {Object} body - Request body data
 * @returns {Promise<Object>} Response data
 */
async function apiPost(url, body) {
    return apiRequest(url, {
        method: 'POST',
        body: JSON.stringify(body)
    });
}

/**
 * DELETE request helper
 * @param {string} url - API endpoint URL
 * @returns {Promise<Object>} Response data
 */
async function apiDelete(url) {
    return apiRequest(url, {
        method: 'DELETE'
    });
}

// ============================================================================
// URL Parameter Utilities
// ============================================================================

/**
 * Get URL parameter value
 * @param {string} param - Parameter name
 * @returns {string|null} Parameter value or null if not found
 */
function getUrlParam(param) {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get(param);
}

/**
 * Get all URL parameters as object
 * @returns {Object} All URL parameters as key-value pairs
 */
function getAllUrlParams() {
    const urlParams = new URLSearchParams(window.location.search);
    const params = {};
    for (const [key, value] of urlParams.entries()) {
        params[key] = value;
    }
    return params;
}

// ============================================================================
// Debounce Utility
// ============================================================================

/**
 * Debounce function execution
 * @param {Function} func - Function to debounce
 * @param {number} delay - Delay in milliseconds (default: 300)
 * @returns {Function} Debounced function
 */
function debounce(func, delay = 300) {
    let timeoutId = null;

    return function(...args) {
        if (timeoutId) {
            clearTimeout(timeoutId);
        }

        timeoutId = setTimeout(() => {
            func.apply(this, args);
        }, delay);
    };
}

// ============================================================================
// localStorage Utilities
// ============================================================================

/**
 * Get item from localStorage with JSON parsing
 * @param {string} key - Storage key
 * @param {*} defaultValue - Default value if key not found
 * @returns {*} Parsed value or default value
 */
function getStorage(key, defaultValue = null) {
    try {
        const item = localStorage.getItem(key);
        return item ? JSON.parse(item) : defaultValue;
    } catch (error) {
        console.warn(`Failed to get storage item: ${key}`, error);
        return defaultValue;
    }
}

/**
 * Set item in localStorage with JSON stringification
 * @param {string} key - Storage key
 * @param {*} value - Value to store
 * @returns {boolean} Success status
 */
function setStorage(key, value) {
    try {
        localStorage.setItem(key, JSON.stringify(value));
        return true;
    } catch (error) {
        console.warn(`Failed to set storage item: ${key}`, error);
        return false;
    }
}

/**
 * Remove item from localStorage
 * @param {string} key - Storage key
 */
function removeStorage(key) {
    try {
        localStorage.removeItem(key);
    } catch (error) {
        console.warn(`Failed to remove storage item: ${key}`, error);
    }
}

// ============================================================================
// Display Utilities
// ============================================================================

/**
 * Show temporary notification
 * @param {string} message - Notification message
 * @param {string} type - Notification type ('success', 'error', 'info')
 * @param {number} duration - Display duration in milliseconds (default: 2000)
 */
function showNotification(message, type = 'success', duration = 2000) {
    // Create notification element if it doesn't exist
    let notification = document.getElementById('hoflix-notification');

    if (!notification) {
        notification = document.createElement('div');
        notification.id = 'hoflix-notification';
        notification.style.cssText = `
            position: fixed;
            bottom: 30px;
            right: 30px;
            background: rgba(0, 0, 0, 0.9);
            color: #fff;
            padding: 12px 20px;
            border-radius: 4px;
            box-shadow: 0 4px 16px rgba(0, 0, 0, 0.4);
            opacity: 0;
            transition: opacity 0.3s;
            z-index: 9999;
            font-size: 14px;
            display: flex;
            align-items: center;
            gap: 10px;
        `;
        document.body.appendChild(notification);
    }

    // Set border color based on type
    const colors = {
        success: '#46d369',
        error: '#E50914',
        info: '#0071eb'
    };

    const icons = {
        success: '✓',
        error: '✗',
        info: 'ℹ'
    };

    notification.style.borderLeft = `4px solid ${colors[type] || colors.info}`;
    notification.innerHTML = `<span style="color: ${colors[type] || colors.info}; font-weight: bold; font-size: 16px;">${icons[type] || icons.info}</span>${message}`;

    // Show notification
    notification.style.opacity = '1';

    // Hide after duration
    setTimeout(() => {
        notification.style.opacity = '0';
    }, duration);
}

// ============================================================================
// Console Log Helpers (for debugging)
// ============================================================================

/**
 * Pretty console log with prefix
 * @param {string} message - Log message
 * @param {*} data - Optional data to log
 */
function log(message, data = null) {
    const prefix = '[Hoflix]';
    if (data !== null) {
        console.log(`${prefix} ${message}`, data);
    } else {
        console.log(`${prefix} ${message}`);
    }
}

/**
 * Error console log with prefix
 * @param {string} message - Error message
 * @param {*} error - Optional error object
 */
function logError(message, error = null) {
    const prefix = '[Hoflix Error]';
    if (error !== null) {
        console.error(`${prefix} ${message}`, error);
    } else {
        console.error(`${prefix} ${message}`);
    }
}
