-- Symbol name
local names =
{
	en = "Lamp",
	cs = "Světelný zdroj"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -1, 2, 1}

-- Terminal points
local terminals = {{-2, 0}, {2, 0}}

-- Rendering
local render = function (cr)
	-- The circle
	cr.save ()

	cr.arc (0, 0, 1, 0, 2 * math.pi)
	cr.stroke_preserve ()
	cr.clip ()

	cr.move_to (-1, -1)
	cr.line_to (1, 1)

	cr.move_to (1, -1)
	cr.line_to (-1, 1)

	cr.stroke ()
	cr.restore ()

	-- The terminals
	cr.move_to (-2, 0)
	cr.line_to (-1, 0)

	cr.move_to (1, 0)
	cr.line_to (2, 0)

	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Lamp", names, area, terminals, render)


