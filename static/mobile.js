function zoomOutOnMobile() {
    if (/Mobi|Android/i.test(navigator.userAgent)) {
        const mainDiv = document.querySelector('main') || document.querySelector('#main') || document.querySelector('.main');

        if (!mainDiv) {
            console.warn('Main div not found');
            return;
        }

        const viewportWidth = window.innerWidth;
        const viewportHeight = window.innerHeight;
        const divWidth = mainDiv.scrollWidth;
        const divHeight = mainDiv.scrollHeight;

        // Calculate scale factor to fit the main div in viewport
        const scaleFactor = Math.min(viewportWidth / divWidth, viewportHeight / divHeight);

        // Apply zoom using transform
        mainDiv.style.transform = `scale(${scaleFactor})`;
        mainDiv.style.transformOrigin = "top left";
        mainDiv.style.width = `${100 / scaleFactor}%`;
        mainDiv.style.height = `${100 / scaleFactor}%`;

        // Allow scrolling for swipe-to-refresh
        document.body.style.overflow = 'auto';
        document.body.style.overscrollBehavior = 'auto';
    }
}

// Only run on initial load
window.addEventListener('load', zoomOutOnMobile);

// Add viewport meta tag to prevent user scaling but allow refresh
const metaViewport = document.createElement('meta');
metaViewport.name = 'viewport';
metaViewport.content = 'width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no';
document.head.appendChild(metaViewport);
