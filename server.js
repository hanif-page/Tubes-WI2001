// REFERENCES structure and code:
// https://github.com/hanif-page/cTask

if(process.env.NODE_ENV !== "production") 
{
    require("dotenv").config()
}

const express = require("express")
const expressLayouts = require("express-ejs-layouts")

const app = express()
const port = process.env.PORT || 3000

app.set('view engine', 'ejs')
app.set('layout', 'main-layout/layout')
app.use(expressLayouts)
app.use(express.static('public'))
// app.use(express.json())
// app.use(methodOverride("_method"))
// app.use(bodyParser.urlencoded({limit: "10mb", extended: false}))

// Requiring router as a middleware
const indexRoute = require("./routes/index")

// Use the router middleware
app.use("/", indexRoute)

app.listen(port, () => console.log(`\nServer running on http://localhost:${port}\n`))