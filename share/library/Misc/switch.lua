-- Symbol name
local names =
{
	en = "Switch",
	cs = "Spínač"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -1.5, 2, 0}

-- Terminals
local terminals = {{-2, 0}, {2, 0}}

-- Rendering
local render_normal = function (cr)
	-- The switch contact
	cr.move_to (1.3, -1.3)
	cr.line_to (-1, 0)

	-- The contacts
	cr.move_to (-2, 0)
	cr.line_to (-1, 0)

	cr.move_to (1, 0)
	cr.line_to (2, 0)

	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Switch", names, area, terminals, render_normal)


