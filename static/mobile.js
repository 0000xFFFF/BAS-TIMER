function zoomOutOnMobile() {
    if (/Mobi|Android/i.test(navigator.userAgent)) {
        const body = document.body;
        const html = document.documentElement;

        const pageWidth = Math.max(body.scrollWidth, html.scrollWidth);
        const pageHeight = Math.max(body.scrollHeight, html.scrollHeight);

        const viewportWidth = window.innerWidth;
        const viewportHeight = window.innerHeight;

        // Calculate scale factor so the entire page fits in viewport
        const scaleFactor = Math.min(viewportWidth / pageWidth, viewportHeight / pageHeight);

        // Apply zoom using transform
        document.body.style.transform = `scale(${scaleFactor})`;
        document.body.style.transformOrigin = "top left";

        // Adjust body width to prevent clipping
        document.body.style.width = `${100 / scaleFactor}%`;
        document.body.style.height = `${100 / scaleFactor}%`;
    }
}

window.addEventListener('load', zoomOutOnMobile);
window.addEventListener('resize', zoomOutOnMobile);
