document.addEventListener('DOMContentLoaded', () => {

    // 1. Select all necessary elements from the DOM
    const mapContainer = document.getElementById('map-container');
    const allPoints = document.querySelectorAll('.map-point');
    
    // --- UPDATED SELECTORS to match your new partial ---
    const floatingBox = document.getElementById('floating-box');
    const boxTitle = document.getElementById('floating-box-title');
    const boxAvailable = document.getElementById('floating-box-available');
    const boxOccupied = document.getElementById('floating-box-occupied');
    const boxLink = document.getElementById('floating-box-link');
    // --- End of updated selectors ---

    // Assume 'buildingStats' object is available from a <script> tag as in the previous answer
    /* Example:
       const buildingStats = {
         "GKU 1": { "available": 3, "occupied": 3 },
         "ASRAMA TB.1": { "available": 17, "occupied": 12 }
       };
    */

    // 2. Add a click event listener to each map point
    allPoints.forEach(point => {
        point.addEventListener('click', (event) => {
            event.stopPropagation(); // Prevents map click from hiding the box immediately

            const buildingName = point.dataset.building;
            const stats = buildingStats[buildingName];

            if (!stats) return;

            // --- UPDATE BOX CONTENT ---
            boxTitle.textContent = buildingName;
            boxAvailable.textContent = stats.available;
            boxOccupied.textContent = stats.occupied;
            const buildingUrl = buildingName.toLowerCase().replace(/[.\s]+/g, '-');
            boxLink.href = `/${buildingUrl}`; // e.g., /building/asrama-tb-1

            // --- POSITION THE BOX ---
            let pointTop = point.offsetTop;
            const pointLeft = point.offsetLeft;
            if(window.innerWidth < 500) {
                pointTop -= 80;
            }
            floatingBox.style.top = `${pointTop-67}px`;
            floatingBox.style.left = `${pointLeft}px`;

            // --- SHOW THE BOX ---
            floatingBox.style.opacity = '1';
            floatingBox.style.cursor = 'auto';
        });
    });

    // 3. Hide the box when clicking on the map background
    mapContainer.addEventListener('click', () => {
        floatingBox.style.opacity = '0';
        floatingBox.style.cursor = 'none';
    });

    // 4. Prevent the box from hiding when clicking on the box itself
    floatingBox.addEventListener('click', (event) => {
        event.stopPropagation();
    });
});