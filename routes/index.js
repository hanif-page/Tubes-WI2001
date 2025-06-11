const express = require("express");
const fs = require('fs');
const path = require('path');
const router = express.Router();

// This line is okay for routes that only READ and don't need real-time data,
// but we will avoid using it for any WRITE operations.
let initialDesks = require("../db.json").desks;

router.get('/', (req, res) => {
    // For the main page, it's often acceptable to use the initially loaded data
    // as it's less critical and reduces file reads.
    const processedStats = initialDesks.reduce((acc, desk) => {
        const buildingName = desk.buildingName;
        if (!acc[buildingName]) {
            acc[buildingName] = { available: 0, occupied: 0 };
        }
        if (desk.deskStatus === 'Available') {
            acc[buildingName].available++;
        } else if (desk.deskStatus === 'Occupied') {
            acc[buildingName].occupied++;
        }
        return acc;
    }, {});
    
    let totalAvailable = 0;
    let totalOccupied = 0;
    for (const building in processedStats) {
        totalAvailable += processedStats[building].available;
        totalOccupied += processedStats[building].occupied;
    }

    res.render('index', {
        buildingStats: processedStats,
        totalAvailable: totalAvailable,
        totalOccupied: totalOccupied
    });
});


router.get('/:buildingName', (req, res) => {
    // This route correctly reads the file fresh each time, which is good.
    const dbPath = path.join(__dirname, '../db.json');
    fs.readFile(dbPath, 'utf8', (err, data) => {
        if (err) {
            return res.status(500).send("Error reading database file.");
        }
        const dbData = JSON.parse(data);
        const requestedBuilding = req.params.buildingName.replace(/-/g, ' ');
        const buildingDesks = dbData.desks.filter(desk => desk.buildingName.toLowerCase() === requestedBuilding.toLowerCase());
        const floors = {};
        buildingDesks.forEach(desk => {
            const floor = desk.buildingFloor;
            if (!floors[floor]) floors[floor] = [];
            floors[floor].push(desk);
        });
        const availableCount = buildingDesks.filter(d => d.deskStatus === 'Available').length;
        const occupiedCount = buildingDesks.length - availableCount;

        res.render('place-detail', {
            buildingName: requestedBuilding.toUpperCase(),
            floors: floors,
            availableCount: availableCount,
            occupiedCount: occupiedCount,
            buildingStats: {} // empty object, because we don't use it in this page
        });
    });
});

// API route for AJAX refresh
router.get('/api/building/:buildingName', (req, res) => {
    const dbPath = path.join(__dirname, '../db.json');
    fs.readFile(dbPath, 'utf8', (err, data) => {
        if (err) {
            console.error("Error reading db.json for API:", err);
            return res.status(500).json({ error: "Could not read database file." });
        }
        try {
            const dbData = JSON.parse(data);
            const requestedBuilding = req.params.buildingName.replace(/-/g, ' ');
            const buildingDesks = dbData.desks.filter(desk => desk.buildingName.toLowerCase() === requestedBuilding.toLowerCase());
            const floors = {};
            buildingDesks.forEach(desk => {
                const floor = desk.buildingFloor;
                if (!floors[floor]) floors[floor] = [];
                floors[floor].push(desk);
            });
            const availableCount = buildingDesks.filter(d => d.deskStatus === 'Available').length;
            const occupiedCount = buildingDesks.length - availableCount;

            res.json({
                buildingName: requestedBuilding.toUpperCase(),
                floors: floors,
                availableCount: availableCount,
                occupiedCount: occupiedCount
            });
        } catch (parseErr) {
            console.error("Error parsing db.json:", parseErr);
            return res.status(500).json({ error: "Database file is corrupted." });
        }
    });
});

// This API endpoint provides the processed statistics for all buildings.
router.get('/api/all-building-stats', (req, res) => {
    const dbPath = path.join(__dirname, '../db.json');

    // Read the latest version of the database file to ensure data is current.
    fs.readFile(dbPath, 'utf8', (err, data) => {
        if (err) {
            console.error("API Error: Could not read db.json.", err);
            return res.status(500).json({ error: "Failed to read database." });
        }

        try {
            const db = JSON.parse(data);

            // --- Process stats for each building ---
            const processedStats = db.desks.reduce((acc, desk) => {
                const buildingName = desk.buildingName;
                if (!acc[buildingName]) {
                    acc[buildingName] = { available: 0, occupied: 0 };
                }
                if (desk.deskStatus === 'Available') {
                    acc[buildingName].available++;
                } else {
                    acc[buildingName].occupied++;
                }
                return acc;
            }, {});

            // --- Calculate overall totals ---
            let totalAvailable = 0;
            let totalOccupied = 0;
            for (const building in processedStats) {
                totalAvailable += processedStats[building].available;
                totalOccupied += processedStats[building].occupied;
            }

            // --- Send the final data as a JSON response ---
            res.json({
                buildingStats: processedStats,
                totalAvailable: totalAvailable,
                totalOccupied: totalOccupied
            });

        } catch (parseErr) {
            console.error("API Error: Could not parse db.json.", parseErr);
            res.status(500).json({ error: "Database file is corrupted." });
        }
    });
});

// --- CORRECTED UPDATE ROUTE ---
router.post('/update-desk', (req, res) => {
    const { buildingName, deskID, isOccupied } = req.body;
    const dbPath = path.join(__dirname, '../db.json');

    // STEP 1: READ the latest version of the file first.
    fs.readFile(dbPath, 'utf8', (err, data) => {
        if (err) {
            console.error("Read error:", err);
            return res.status(500).send("Error reading database file before update.");
        }

        try {
            const db = JSON.parse(data);
            
            // STEP 2: MODIFY the data that was just read.
            const deskIndex = db.desks.findIndex(d => d.deskID == deskID && d.buildingName == buildingName);

            if (deskIndex !== -1) {
                db.desks[deskIndex].deskStatus = isOccupied ? 'Occupied' : 'Available';
                
                // STEP 3: WRITE the modified data back to the file.
                fs.writeFile(dbPath, JSON.stringify(db, null, 4), (writeErr) => {
                    if (writeErr) {
                        console.error("Write error:", writeErr);
                        return res.status(500).send("Error writing updated data to database file.");
                    }
                    console.log(`Successfully updated Desk ID ${deskID}`);
                    // Also update the in-memory version for the main page after a successful write
                    initialDesks = db.desks;
                    res.status(200).send({ message: `Desk ${deskID} updated.` });
                });
            } else {
                res.status(404).send({ message: `Desk with ID ${deskID} not found.` });
            }
        } catch (parseErr) {
            console.error("Parse error:", parseErr);
            res.status(500).send("Database file is corrupt, cannot update.");
        }
    });
});


module.exports = router;
