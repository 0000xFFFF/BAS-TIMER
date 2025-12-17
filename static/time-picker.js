(function() {
    function initTimePicker(picker) {
        picker.querySelectorAll('.tp-unit').forEach(unit => {
            const valueEl = unit.querySelector('.tp-value');
            const max = parseInt(unit.dataset.max, 10);
            let value = parseInt(valueEl.textContent, 10) || 0;

            const render = () => {
                valueEl.textContent = String(value).padStart(2, '0');
            };

            const increment = () => {
                value = (value + 1) % (max + 1);
                render();
            };

            const decrement = () => {
                value = (value - 1 + (max + 1)) % (max + 1);
                render();
            };

            // Click upper / lower half
            unit.addEventListener('click', e => {
                const rect = unit.getBoundingClientRect();
                const y = e.clientY - rect.top;
                y < rect.height / 2 ? increment() : decrement();
            });

            // Mouse wheel
            unit.addEventListener(
                'wheel',
                e => {
                    e.preventDefault();
                    e.deltaY < 0 ? increment() : decrement();
                },
                { passive: false }
            );

            render();
        });
    }

    document.addEventListener('DOMContentLoaded', () => {
        document
            .querySelectorAll('.time-picker')
            .forEach(initTimePicker);
    });

    window.getTimePickerValue = function(pickerId) {
        const picker = document.getElementById(pickerId);
        if (!picker) return null;

        return [...picker.querySelectorAll('.tp-value')]
            .map(v => v.textContent)
            .join(':');
    };


    // getTimePickerValue("time-picker-1"); // "08:15:30"
    // getTimePickerValue("time-picker-2"); // "22:00:00"
})();

