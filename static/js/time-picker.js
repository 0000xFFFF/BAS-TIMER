(function() {
    function initTimePicker(picker) {
        picker.querySelectorAll('.tp_unit').forEach(unit => {
            const valueEl = unit.querySelector('.tp_value');
            const max = parseInt(unit.dataset.max, 10);
            let value = parseInt(valueEl.textContent, 10) || 0;

            const render = () => {
                valueEl.textContent = String(value).padStart(2, '0');

                picker.dispatchEvent(
                    new CustomEvent('timechange', {
                        detail: getTimePickerValue(picker.id)
                    })
                );
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

    window.initAllTimePickers = function() {
        document.querySelectorAll('.time_picker').forEach(initTimePicker);
    }

    window.getTimePickerValueStr = function(pickerId) {
        const picker = document.getElementById(pickerId);
        if (!picker) return null;

        return [...picker.querySelectorAll('.tp_value')]
            .map(v => v.textContent)
            .join(':');
    };

    window.getTimePickerValue = function(pickerId) {
        const picker = document.getElementById(pickerId);
        if (!picker) return null;

        const values = picker.querySelectorAll('.tp_value')
        return parseInt(values[0].textContent) * 3600 + parseInt(values[1].textContent) * 60 + parseInt(values[2].textContent);
    };
    // getTimePickerValue("time_picker_1"); // "08:15:30"
    // getTimePickerValue("time_picker_2"); // "22:00:00"


    window.setTimePickerFromSeconds = function(pickerId, totalSeconds) {
        const picker = document.getElementById(pickerId);
        if (!picker) return;

        totalSeconds = ((totalSeconds % 86400) + 86400) % 86400; // normalize

        const h = Math.floor(totalSeconds / 3600);
        const m = Math.floor((totalSeconds % 3600) / 60);
        const s = totalSeconds % 60;

        const values = [h, m, s];
        picker.querySelectorAll('.tp_value').forEach((el, i) => {
            el.textContent = String(values[i]).padStart(2, '0');
        });
    };

    window.setTimePickerToNow = function(pickerId) {
        const picker = document.getElementById(pickerId);
        if (!picker) return false;

        const now = new Date();
        const values = [
            now.getHours(),
            now.getMinutes(),
            now.getSeconds()
        ];

        const units = picker.querySelectorAll('.tp_unit');

        units.forEach((unit, i) => {
            const valueEl = unit.querySelector('.tp_value');
            if (!valueEl) return;

            valueEl.textContent = String(values[i]).padStart(2, '0');
        });

        return true;
    };

    // setTimePickerToNow("schedules_timer_picker_3");


})();

