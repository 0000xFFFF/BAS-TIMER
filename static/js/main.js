async function embed_innerHTML(id, url) {
    return fetch(url)
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.text();
        })
        .then(html => {
            document.getElementById(id).innerHTML = html;
        })
        .catch(err => {
            console.error('Failed to embed/load HTML:', err);
        });

}
