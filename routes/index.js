const express = require("express")
const router = express.Router()

// creating a dummy desk data
let desks = require("../db.json").desks

router.get('/', (req, res) => {
    res.render("index", {
        desks: desks
    })
})

router.get('/gku-1', (req, res) => {
    res.render("place-detail", {
        desks: desks
    })
})

router.get('/gku-1/:deskID', (req, res) => {
    res.render("place-detail", {
        deskID: req.params.deskID,
        desks: desks
    })
})

// const getUser = async (req) => {
//     const userEmail = req.user.email
//     const user = await User.findOne({ email: userEmail })

//     return user // will be mainly use as boolean statement (either true or false)
// }

module.exports = router