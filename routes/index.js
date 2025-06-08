const express = require("express")
const router = express.Router()

// creating a dummy desk data
let desks = require("../db.json").desks

router.get('/', (req, res) => {
// --- 1. PROCESS THE DATA ---
    // We will use the .reduce() method to transform the array of desks 
    // into an object of buildings with their stats.
    const processedStats = desks.reduce((acc, desk) => {
        const buildingName = desk.buildingName;

        // If we haven't seen this building yet, initialize it in our object
        if (!acc[buildingName]) {
            acc[buildingName] = {
                available: 0,
                occupied: 0
            };
        }

        // Increment the count based on the desk's status
        if (desk.deskStatus === 'Available') {
            acc[buildingName].available++;
        } else if (desk.deskStatus === 'Occupied') {
            acc[buildingName].occupied++;
        }

        return acc; // Return the accumulator for the next iteration
    }, {}); // The initial value of our accumulator is an empty object {}

    /*
    After the reduce function, `processedStats` will look like this:
    {
      "GKU 1": { "available": 3, "occupied": 3 },
      "GKU 2": { "available": 2, "occupied": 2 }
    }
    */
    
    // --- 2. CALCULATE TOTALS for the bottom bar ---
    let totalAvailable = 0;
    let totalOccupied = 0;
    for (const building in processedStats) {
        totalAvailable += processedStats[building].available;
        totalOccupied += processedStats[building].occupied;
    }


    // --- 3. RENDER THE EJS TEMPLATE ---
    // Pass the processed data to the 'index.ejs' file.
    res.render('index', {
        buildingStats: processedStats, // This object will be used for the floating box
        totalAvailable: totalAvailable,  // This number is for the bottom bar
        totalOccupied: totalOccupied     // This number is for the bottom bar
    });
})

// router.get('/gku-1', (req, res) => {
//     // getting the available count and the occupied count
//     let availableCount = 0;
//     let occupiedCount = 0;

//     desks.forEach(el => {
//         if(el.buildingName == "GKU 1" && el.deskStatus == "Available") {
//             availableCount += 1;
//             console.log("available");
//         }
//         else if (el.buildingName == "GKU 1" && el.deskStatus == "Occupied"){
//             occupiedCount += 1;
//             console.log("occupied");
//         }
//     });

//     res.render("place-detail", {
//         desks: desks,
//         buildingName: "GKU 1",
//         availableCount,
//         occupiedCount
//     })
// })

router.get('/:buildingName', (req, res) => {
    const requestedBuilding = req.params.buildingName.replace('-', ' ');
    const dbData = require("../db.json"); // Load the JSON data

    // 1. Filter desks for the requested building
    const buildingDesks = dbData.desks.filter(desk => desk.buildingName.toLowerCase() === requestedBuilding.toLowerCase());

    // 2. Group desks by floor
    const floors = {};
    buildingDesks.forEach(desk => {
        const floor = desk.buildingFloor;
        if (!floors[floor]) {
            floors[floor] = [];
        }
        floors[floor].push(desk);
    });

    // 3. Calculate available and occupied counts
    const availableCount = buildingDesks.filter(d => d.deskStatus === 'Available').length;
    const occupiedCount = buildingDesks.length - availableCount;

    // 4. Render the EJS template with the prepared data
    res.render('place-detail', {
        buildingName: requestedBuilding.toUpperCase(),
        floors: floors,
        availableCount: availableCount,
        occupiedCount: occupiedCount,
        buildingStats: {} // empty object, because we don't use it in this page
    });
});

router.get('/gku-1/:deskID', (req, res) => {
    res.render("place-detail", {
        deskID: req.params.deskID,
        desks: desks
    })
})

router.get('/tb-1', (req, res) => {
    // getting the available count and the occupied count
    let availableCount = 0;
    let occupiedCount = 0;

    desks.desks.forEach(el => {
        
    });

    res.render("place-detail", {
        desks: desks,
        buildingName: "Asrama TB-1"
    })
})

// const getUser = async (req) => {
//     const userEmail = req.user.email
//     const user = await User.findOne({ email: userEmail })

//     return user // will be mainly use as boolean statement (either true or false)
// }

module.exports = router